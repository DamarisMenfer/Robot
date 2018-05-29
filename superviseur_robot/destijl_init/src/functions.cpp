#include "../header/functions.h"

char mode_start;

void write_in_queue(RT_QUEUE *, MessageToMon);
void send_compressed_img(Image *img);

Camera cam; // /!\ STUPID
Arene arene;
bool arenaConfirmed = false;

void f_server(void *arg) {
    int err;
    /* INIT */
    RT_TASK_INFO info;
    rt_task_inquire(NULL, &info);
    printf("Init %s\n", info.name);
    rt_sem_p(&sem_barrier, TM_INFINITE);

    err = run_nodejs("/usr/local/bin/node", "/home/pi/Interface_Robot/server.js");

    if (err < 0) {
        printf("Failed to start nodejs: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    } else {
#ifdef _WITH_TRACE_
        printf("%s: nodejs started\n", info.name);
#endif
        open_server();
        rt_sem_broadcast(&sem_serverOk);
    }
}

int cpt_perte_co = 0;

void f_sendToMon(void * arg) {
    MessageToMon msg;

    /* INIT */
    RT_TASK_INFO info;
    rt_task_inquire(NULL, &info);
    printf("Init %s\n", info.name);
    rt_sem_p(&sem_barrier, TM_INFINITE);

#ifdef _WITH_TRACE_
    printf("%s : waiting for sem_serverOk\n", info.name);
#endif
    rt_sem_p(&sem_serverOk, TM_INFINITE);
    cpt_perte_co = 0;
    while (cpt_perte_co < 50) {
#ifdef _WITH_TRACE_
        printf("%s : waiting for a message in queue\n", info.name);
#endif
        if (rt_queue_read(&q_messageToMon, &msg, sizeof (MessageToRobot), TM_INFINITE) >= 0) {
#ifdef _WITH_TRACE_
            printf("%s : message {%s,%s} in queue\n", info.name, msg.header, msg.data);
#endif
            int err = send_message_to_monitor(msg.header, msg.data);
            free_msgToMon_data(&msg);
            rt_queue_free(&q_messageToMon, &msg);
            if(err <= 0){
                cpt_perte_co ++;
                if(cpt_perte_co>50){
                    printf("Connexion perdu\n");
                    kill_nodejs();
                    close_communication_robot();
                    //arreter la cam, le show est terminé
                }
            }
            else{
                cpt_perte_co = 0;
            }
            
        } else {
            printf("Error msg queue write: \n");
        }
    }
}

void f_receiveFromMon(void *arg) {
    MessageFromMon msg;
    int err = 1;

    /* INIT */
    RT_TASK_INFO info;
    rt_task_inquire(NULL, &info);
    printf("Init %s\n", info.name);
    rt_sem_p(&sem_barrier, TM_INFINITE);

#ifdef _WITH_TRACE_
    printf("%s : waiting for sem_serverOk\n", info.name);
#endif
    rt_sem_p(&sem_serverOk, TM_INFINITE);
    do {
#ifdef _WITH_TRACE_
        printf("%s : waiting for a message from monitor\n", info.name);
#endif
        err = receive_message_from_monitor(msg.header, msg.data);
#ifdef _WITH_TRACE_
        printf("%s: msg {header:%s,data=%s} received from UI\n", info.name, msg.header, msg.data);
#endif
        if (strcmp(msg.header, HEADER_MTS_COM_DMB) == 0) {
            if (msg.data[0] == OPEN_COM_DMB) { // Open communication supervisor-robot
#ifdef _WITH_TRACE_
                printf("%s: message open Xbee communication\n", info.name);
#endif
                rt_sem_v(&sem_openComRobot);
            }
        } else if (strcmp(msg.header, HEADER_MTS_DMB_ORDER) == 0) {
            if (msg.data[0] == DMB_START_WITHOUT_WD) { // Start robot
#ifdef _WITH_TRACE_
                printf("%s: message start robot\n", info.name);
#endif          
                rt_sem_v(&sem_startRobot);

            } else if ((msg.data[0] == DMB_GO_BACK)
                    || (msg.data[0] == DMB_GO_FORWARD)
                    || (msg.data[0] == DMB_GO_LEFT)
                    || (msg.data[0] == DMB_GO_RIGHT)
                    || (msg.data[0] == DMB_STOP_MOVE)) {

                rt_mutex_acquire(&mutex_move, TM_INFINITE);
                move = msg.data[0];
                rt_mutex_release(&mutex_move);
#ifdef _WITH_TRACE_
                printf("%s: message update movement with %c\n", info.name, move);
#endif
            }
        } else if (strcmp(msg.header, HEADER_MTS_CAMERA) == 0) {
            send_message_to_monitor(HEADER_STM_ACK, NULL);
            
            // Open camera
            if (msg.data[0] == CAM_OPEN) {
                if (open_camera(&cam) != 0) {
                    send_message_to_monitor(HEADER_STM_MES, "Failed opening camera\n");
                } else {
                    // Start task
                    int err2;
                    if (err2 = rt_task_start(&th_sendImage, &f_sendImage, NULL)) {
                        printf("Error task start: %s\n", strerror(-err2));
                        exit(EXIT_FAILURE);
                    }
                }
            }
            // Close camera
            else if (msg.data[0] == CAM_CLOSE) {
                //send_message_to_monitor(HEADER_STM_ACK, NULL);
                send_message_to_monitor(HEADER_STM_MES, "Closing camera...\n");
                printf("Closing camera...\n");
                close_camera(&cam);
            }
            // Ask arena
            else if (msg.data[0] == CAM_ASK_ARENA) {
                //send_message_to_monitor(HEADER_STM_ACK, NULL);
                send_message_to_monitor(HEADER_STM_MES, "Asking arena...\n");
                printf("Asking arena...\n");
                
                // Get image
                Image imgIn, imgOut;
                get_image(&cam, &imgIn);
                if (detect_arena(&imgIn, &arene) != 0) // TODO: global var!
                    printf("Failed detecting arena\n");
                printf("Drawing arena...\n");
                draw_arena(&imgIn, &imgOut, &arene);
                send_compressed_img(&imgOut);

                // Suspend sendImage task
                rt_task_suspend(&th_sendImage);
            }
            // Confirm arena
            else if (msg.data[0] == CAM_ARENA_CONFIRM) {
                printf("Arena confirmed\n");
                arenaConfirmed = true;
                rt_task_resume(&th_sendImage);
            }
            // Infirm arena
            else if (msg.data[0] == CAM_ARENA_INFIRM) {
                printf("Arena infirmed\n");
                arenaConfirmed = false;
                rt_task_resume(&th_sendImage);
            }
        }
    } while (err > 0);

}

void f_openComRobot(void * arg) {
    int err;

    /* INIT */
    RT_TASK_INFO info;
    rt_task_inquire(NULL, &info);
    printf("Init %s\n", info.name);
    rt_sem_p(&sem_barrier, TM_INFINITE);

    while (1) {
#ifdef _WITH_TRACE_
        printf("%s : Wait sem_openComRobot\n", info.name);
#endif
        rt_sem_p(&sem_openComRobot, TM_INFINITE);
#ifdef _WITH_TRACE_
        printf("%s : sem_openComRobot arrived => open communication robot\n", info.name);
#endif
        err = open_communication_robot();
        if (err == 0) {
#ifdef _WITH_TRACE_
            printf("%s : the communication is opened\n", info.name);
#endif
            MessageToMon msg;
            set_msgToMon_header(&msg, HEADER_STM_ACK);
            write_in_queue(&q_messageToMon, msg);
        } else {
            MessageToMon msg;
            set_msgToMon_header(&msg, HEADER_STM_NO_ACK);
            write_in_queue(&q_messageToMon, msg);
        }
    }
}

void f_startRobot(void * arg) {
    int err;

    /* INIT */
    RT_TASK_INFO info;
    rt_task_inquire(NULL, &info);
    printf("Init %s\n", info.name);
    rt_sem_p(&sem_barrier, TM_INFINITE);

    while (1) {
#ifdef _WITH_TRACE_
        printf("%s : Wait sem_startRobot\n", info.name);
#endif
        rt_sem_p(&sem_startRobot, TM_INFINITE);
#ifdef _WITH_TRACE_
        printf("%s : sem_startRobot arrived => Start robot\n", info.name);
#endif
        err = send_command_to_robot(DMB_START_WITHOUT_WD);
        if (err == 0) {
#ifdef _WITH_TRACE_
            printf("%s : the robot is started\n", info.name);
#endif
            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
            robotStarted = 1;
            rt_mutex_release(&mutex_robotStarted);
            MessageToMon msg;
            set_msgToMon_header(&msg, HEADER_STM_ACK);
            write_in_queue(&q_messageToMon, msg);
        } else {
            MessageToMon msg;
            set_msgToMon_header(&msg, HEADER_STM_NO_ACK);
            write_in_queue(&q_messageToMon, msg);
        }
    }
}

void f_battery(void  *arg){
     /* INIT */
    RT_TASK_INFO info;
    rt_task_inquire(NULL, &info);
    printf("Init %s\n", info.name);
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    rt_task_set_periodic(NULL, TM_NOW, 500000000);
    
    while(1){
        printf("Je suis là\n");
        rt_task_wait_period(NULL);
        int temp = 0;
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        if (robotStarted) {
            temp = send_command_to_robot(DMB_GET_VBAT);
            MessageToMon msg;
            temp += 48;
            set_msgToMon_header(&msg, HEADER_STM_BAT);
            set_msgToMon_data(&msg, &temp);
            write_in_queue(&q_messageToMon, msg);
        }
        rt_mutex_release(&mutex_robotStarted);    
        
    }
}

void f_move(void *arg) {
    /* INIT */
    RT_TASK_INFO info;
    rt_task_inquire(NULL, &info);
    printf("Init %s\n", info.name);
    rt_sem_p(&sem_barrier, TM_INFINITE);

    /* PERIODIC START */
#ifdef _WITH_TRACE_
    printf("%s: start period\n", info.name);
#endif
    rt_task_set_periodic(NULL, TM_NOW, 100000000);
    while (1) {
#ifdef _WITH_TRACE_
        printf("%s: Wait period \n", info.name);
#endif
        rt_task_wait_period(NULL);
#ifdef _WITH_TRACE_
       printf("%s: Periodic activation\n", info.name);
        printf("%s: move equals %c\n", info.name, move);
#endif
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        if (robotStarted) {
            rt_mutex_acquire(&mutex_move, TM_INFINITE);
            send_command_to_robot(move);
            rt_mutex_release(&mutex_move);

#ifdef _WITH_TRACE_
            printf("%s: the movement %c was sent\n", info.name, move);
#endif            
        }
        rt_mutex_release(&mutex_robotStarted);
    }
}

void f_sendImage(void *arg)
{
     
    RT_TASK_INFO info;
    rt_task_inquire(NULL, &info);
    printf("Init %s\n", info.name);
    //rt_sem_p(&sem_barrier, TM_INFINITE);
    
    rt_task_set_periodic(NULL, TM_NOW, 10000000);
    
    while(1) {
        //printf("sendImage task\n");
        rt_task_wait_period(NULL);
        
        // Get image
        Image img;
        get_image(&cam, &img);

        // Draw arena if confirmed
        Image img2 = img.clone();
        if (arenaConfirmed)
            draw_arena(&img, &img2, &arene);
       
        send_compressed_img(&img2);
    }
}

void f_position(void * arg){
         /* INIT */
    RT_TASK_INFO info;
    rt_task_inquire(NULL, &info);
    printf("Init %s\n", info.name);
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    rt_task_set_periodic(NULL, TM_NOW, 100000000);
    rt_sem_p(&sem_position, TM_INFINITE);
    while(1){
        rt_task_wait_period(NULL);
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        if (robotStarted) {
            printf("Position\n");
            Camera camer;
            Image im;
            Image imWithPos;
            Position pos;
            open_camera(&camer);
            get_image(&camer, &im);
            detect_position(&im, &pos);
            draw_position(&im, &imWithPos, &pos);
        }
        rt_mutex_release(&mutex_robotStarted);    
        
    }
    Camera camer;
    open_camera(&camer);
}

void write_in_queue(RT_QUEUE *queue, MessageToMon msg) {
    void *buff;
    buff = rt_queue_alloc(&q_messageToMon, sizeof (MessageToMon));
    memcpy(buff, &msg, sizeof (MessageToMon));
    rt_queue_send(&q_messageToMon, buff, sizeof (MessageToMon), Q_NORMAL);
}

void send_compressed_img(Image *img)
{
    Jpg img_compressed;
    compress_image(img, &img_compressed);
    //printf("Img compressed\n");

    // Send image to monitor
    MessageToMon msg;
    send_message_to_monitor(HEADER_STM_IMAGE, &img_compressed);;
    //printf("Img sent\n");
}

