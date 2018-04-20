#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
    /* 输入的命令行 */
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    int pipe_num;
    int i;
    while (1) {
        /* 提示符 */
        printf("# ");
        pipe_num=0;
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        for (i = 0; cmd[i] != '\n'; i++);
            
        cmd[i] = '\0';
        /* 拆解命令行 */
        
        for(args[0] = cmd; *args[0]==' '; args[0]++);

        for (i = 0; *args[i]; i++)
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
                if (*args[i+1] == ' ') {
                    *args[i+1] = '\0';
                    args[i+1]++;
                    for(; *args[i+1] == ' '; args[i+1]++);
                    break;
                }
        args[i] = NULL;

        /* 没有输入命令 */
        if (!args[0])
            continue;

        for (i = 0; args[i]; i++)
           if (*args[i] == '|')
                pipe_num++;

        if (pipe_num > 0) {//开始处理管道

            char *ARGS[128][128];
            int t;
            int k;
            int a = 0;
            pid_t PID;
            for (t = 0; args[a] != NULL; t++)//将多个命令及参数拆分
                for (k = 0; args[a] != NULL; k++){
                    if (*args[a] != '|') {
                        ARGS[t][k] = args[a++];
                    }
                    else {
                        args[a++] = NULL;
                        ARGS[t][k] = NULL;
                        break;
                    }
                }
            t--;
            ARGS[t][k]=NULL;
            int fd[100][2];
            for(i=0;i<pipe_num;i++)//创建pipe_num个管道
                if((pipe(fd[i]))==-1){
                    perror("pipe error");
                    exit(1);
                }


            
            for(i=0;i<pipe_num+1;i++){//创建pipe_num+1个进程
                if((PID=fork())==0)
                    break;
                else if(PID==-1){
                    perror("fork error!");
                    exit(1);
                }
            }

            if(i==0){
                for(t=0;t<pipe_num;t++)
                    close(fd[t][0]);
                for(t=0;t<pipe_num;t++){
                    if(t!=i)
                        close(fd[t][1]);
                }
                dup2(fd[i][1],STDOUT_FILENO);
                execvp(ARGS[i][0],ARGS[i]); //第一个子进程
            }
            else if(i>0&&i<pipe_num){
                for(t=0;t<pipe_num;t++){
                    if(t!=i-1)
                        close(fd[t][0]);
                }
                for(t=0;t<pipe_num;t++){
                    if(t!=i)
                        close(fd[t][1]);
                }
                dup2(fd[i-1][0],STDIN_FILENO);
                dup2(fd[i][1],STDOUT_FILENO);
                execvp(ARGS[i][0],ARGS[i]);
            }
            else if(i==pipe_num){
                for(t=0;t<pipe_num;t++){
                    if(t!=i-1)
                        close(fd[t][0]);
                }
                for(t=0;t<pipe_num;t++)
                    close(fd[t][1]);
                dup2(fd[i-1][0],STDIN_FILENO);
                execvp(ARGS[i][0],ARGS[i]);
            }
            else{
                for(t=0;t<pipe_num;t++)
                    close(fd[t][0]);
                for(t=0;t<pipe_num;t++)
                    close(fd[t][1]);
                for(t=0;t<pipe_num+1;t++)
                    wait(NULL);
            }

            continue;





        }


        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                if (chdir(args[1])==-1)
                    printf("路径输入错误！请重新输入\n");
            continue;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            continue;
        }
        if (strcmp(args[0], "exit") == 0)
            return 0;

        if (strcmp(args[0], "export")==0){
            char *value=args[1];
            for (; *value!='=' && *value!='\0'; value++);
            if (*value == '='){
                *value = '\0';
                value++;
                if(setenv(args[1],value,1)==-1){
                    printf("环境变量设置失败，请检查格式！\n");

                }
            }
            else
                printf("环境变量设置失败，请检查格式！\n");
            continue;

        }

        

        /* 外部命令 */
        pid_t pid = fork();
        if (pid == 0) {
            /* 子进程 */
            execvp(args[0], args);
            /* execvp失败 */
            return 255;
        }
        /* 父进程 */
        wait(NULL);
    }
}
