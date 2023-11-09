#ifndef __REQUEST_H__


////////////////////////////////////////// thread struct
typedef struct thread_info{
    int total_requests_handled;
    int total_static_requests_handled;
    int total_dynamic_requests_handled;
}thread_info;

void requestHandle(int fd,int my_thread ,struct timeval arrival,struct timeval interval,struct thread_info* arr);

#endif
