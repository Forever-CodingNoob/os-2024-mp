#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MAX_NAME_LEN 10
#define MAX_REC_DEPTH 4
#define MAX_PATH_LEN (MAX_NAME_LEN*(MAX_REC_DEPTH+1)+MAX_REC_DEPTH+100)

typedef struct{
    int dir_num;
    int file_num;
} Num;

char* fullpath;
char key;
Num* num;

char* fmtname(){
    //static char buf[MAX_PATH_LEN+1];
    //char *p;
    //int root_len = strlen(root_dir);

    //for(p=fullpath; *p != 0 && *p != '/'; p++);
    
    //strcpy(buf, root_dir);
    //strcpy(buf+root_len, p); //include null
    //return buf;
    return fullpath;
}

int count_key(char* str){
    int cnt=0;
    for(;*str;str++) if(*str==key) cnt++;
    return cnt;
}

void _list(int depth, int path_len,int key_occur){
    struct stat statbuf;
    struct dirent dirbuf;
    int fd;

    if((fd=open(fullpath, O_RDONLY))<0){
        printf("%s [error opening dir]\n", fullpath);
        return;
    }

    fstat(fd, &statbuf);
    
    if(depth==0 && statbuf.type!=T_DIR){
        printf("%s [error opening dir]\n", fullpath);
        close(fd);
        return;
    }
        
    printf("%s %d\n", fmtname(), key_occur);
    
    if(depth>0){
        if(statbuf.type != T_DIR){
            if(statbuf.type==T_FILE) num->file_num++;
            close(fd);
            return;
        }
        num->dir_num++;
    }

    fullpath[path_len++]='/';
    fullpath[path_len]=0;

    while(read(fd, &dirbuf, sizeof(dirbuf))==sizeof(dirbuf)){
        if(dirbuf.inum==0 || strcmp(dirbuf.name, ".")==0 || strcmp(dirbuf.name, "..")==0) continue;
        strcpy(fullpath+path_len, dirbuf.name);
        _list(depth+1, path_len+strlen(dirbuf.name), key_occur+count_key(dirbuf.name));
    }
    fullpath[path_len-1]=0;
    close(fd);
}

void ftw(char* path, char _key, Num* count_buf){
    fullpath = (char*)malloc(MAX_PATH_LEN+1);
    // root_dir = path;
    key=_key;
    num=count_buf;
    num->dir_num =0;
    num->file_num =0;
    strcpy(fullpath, path);
    _list(0, strlen(fullpath), count_key(fullpath));
    free(fullpath);
    return;
}

int main(int argc, char* argv[]){
    char* root_dir=argv[1];
    char _key=argv[2][0];
    int fd[2];

    pipe(fd);

    if(fork()==0){
        // child
        close(fd[0]);
        Num* retp = (Num*)malloc(sizeof(Num));
        ftw(root_dir, _key, retp);
        write(fd[1], retp, sizeof(*retp));
        free(retp);
    }else{
        // parent
        close(fd[1]);
        Num ret;
        read(fd[0], &ret, sizeof(ret));
        printf("\n%d directories, %d files\n", ret.dir_num, ret.file_num);
    }
    exit(0);
}
