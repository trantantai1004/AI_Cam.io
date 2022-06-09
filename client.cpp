#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <errno.h>
#include <wiringPi.h>


using namespace cv;

using namespace std;

pthread_t thread1,thread2;
pthread_mutex_t lock1;
int dem=0;

void send_image(int socket_desc)
{
        FILE *picture;
    int size, read_size, stat, packet_index;
    char send_buffer[10240], read_buffer[256];
    packet_index = 1;

    picture = fopen("test.jpg", "r");
    printf("Getting Picture Size\n");
    if(picture == NULL) {
         printf("Error Opening Image File"); }
    fseek(picture, 0, SEEK_END);
    size = ftell(picture);
    fseek(picture, 0, SEEK_SET);
    printf("Total Picture size: %i\n",size);
    //Send Picture Size
    printf("Sending Picture Size\n");
    write(socket_desc, (void *)&size, sizeof(int));
    //Send Picture as Byte Array
    printf("Sending Picture as Byte Array\n");
    do { //Read while we get errors that are due to signals.
       stat=read(socket_desc, &read_buffer , 255);
       printf("Bytes read: %i\n",stat);
    } while (stat < 0);
    printf("Received data in socket\n");
    printf("Socket data: %c\n", read_buffer);
        while(!feof(picture)) {
       //Read from the file into our send buffer
       read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

       //Send data through our socket
       do{
         stat = write(socket_desc, send_buffer, read_size);
       }while (stat < 0);

       printf("Packet Number: %i\n",packet_index);
       printf("Packet Size Sent: %i\n",read_size);
       printf(" \n");
       printf(" \n");


       packet_index++;
//      /Zero out our send buffer
       bzero(send_buffer, sizeof(send_buffer));         

        }
}
void* CaptureFunction(void *arg)
{
        pthread_mutex_lock(&lock1);
        VideoCapture cap(0);
        CascadeClassifier faceCascade;
        faceCascade.load("haarcascade_frontalface_default.xml");
        if (faceCascade.empty())
        {
                cout << "XML not loaded" << endl;
        }
        while (true) {
        //      if(dem == 1){

                //      pthread_mutex_lock(&lock1);
                //}
                Mat image;
                cap >> image;
                vector<Rect> faces;
                faceCascade.detectMultiScale(image, faces, 1.1, 8);
                for (int i = 0;i < faces.size(); i++)
                {
                                        rectangle(image, faces[i].tl(),faces[i].br(), Scalar(25>
                }
                if (faces.size() != 0)
                {
                        cout << "1" <<endl;
                        imwrite("test.jpg", image); // A JPG FILE IS BEING SAVED
                        cout << "Image is saved successfully   ^`   ..";
                        pthread_mutex_unlock(&lock1);
                }
                else 
                { 
                        dem = 0;
                        cout << "0" << endl;
                }
                if(dem == 1){

                       pthread_mutex_lock(&lock1);
                }
        }
}
void* Send(void *arg)
{
//      cout << " Pthread 2 " << endl; 

        while(true){
    int socket_desc;
    struct sockaddr_in server;
    char *parray;


    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);

    if (socket_desc == -1) {
        printf("Could not create socket");
    }

    memset(&server,0,sizeof(server));
    server.sin_addr.s_addr = inet_addr("119.17.253.45");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8889 );

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0){
        cout<<strerror(errno);
        close(socket_desc);
                puts("Connect Error");
//        return 1;
    }

    puts("Connected\n");
        pthread_mutex_lock(&lock1);
        digitalWrite(0,1);
        sleep(3);
        digitalWrite(0,0);
        send_image(socket_desc);   
        dem = 1;
        sleep(5);
        pthread_mutex_unlock(&lock1);


}
        return NULL;
}

int main(){
        wiringPiSetup();
        pinMode(0, OUTPUT);
        digitalWrite(0,0);
        pthread_mutex_init(&lock1, NULL);
        pthread_create(&thread1, NULL, &CaptureFunction, NULL);
        pthread_create(&thread2, NULL, &Send, NULL);
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);
        pthread_mutex_destroy(&lock1);
        return 0;
}




	
