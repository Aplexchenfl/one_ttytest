#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

/*
 * structure aplex_tty:
 * tty_node: serial file node , ex: /dev/ttyO0, /dev/ttyO1;
 * test_count : when test count > test_count , test over , exit; ex: 5000;
 * usleep_time : test speed , How often do you send the data, ex: 100000 (us);
 * test_chr : the data of send to serial ;
 */
struct aplex_tty
{
    char tty_node[16];
    int stop_count;
    unsigned int usleep_time;
    char test_chr[32];
};

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);

void init_aplex_tty(struct aplex_tty *, char *tty_node, int test_count, unsigned int usleep_time, char test_chr[32]);
void test_tty(struct aplex_tty *);

int main(int argc, char *argv[])
{
    struct aplex_tty my_tty;
    int test_count = atoi(argv[2]);
    unsigned int usleep_time = atol(argv[3]);
    char test_chr[32] = "abcd9876543210\0";

    init_aplex_tty(&my_tty, argv[1], test_count, usleep_time, test_chr);

    test_tty(&my_tty);

    return 0;
}

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;
    if(tcgetattr(fd, &oldtio) != 0)
    {
        perror("SetupSerial 1");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;   //CLOCAL:忽略modem控制线  CREAD：打开接受者
    newtio.c_cflag &= ~CSIZE;           //字符长度掩码。取值为：CS5，CS6，CS7或CS8

    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;           //允许输出产生奇偶信息以及输入到奇偶校验
        newtio.c_cflag |= PARODD;           //输入和输出是奇及校验
        newtio.c_iflag |= (INPCK | ISTRIP); // INPACK:启用输入奇偶检测；ISTRIP：去掉第八位
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }

    switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }

    if( nStop == 1 )
        newtio.c_cflag &=  ~CSTOPB;     //CSTOPB:设置两个停止位，而不是一个
    else if ( nStop == 2 )
        newtio.c_cflag |=  CSTOPB;

    newtio.c_cc[VTIME]  = 0;            //VTIME:非cannoical模式读时的延时，以十分之一秒位单位
    newtio.c_cc[VMIN] = 0;              //VMIN:非canonical模式读到最小字符数
    tcflush(fd,TCIFLUSH);               // 改变在所有写入 fd 引用的对象的输出都被传输后生效，所有已接受但未读入的输入都在改变发生前丢弃。
    if((tcsetattr(fd,TCSANOW,&newtio))!=0) //TCSANOW:改变立即发生
    {
        perror("com set error");
        return -1;
    }
    printf("set done!\n\r");
    return 0;
}

void init_aplex_tty(struct aplex_tty *_aplex_tty, char *tty_node, int stop_count, unsigned int usleep_time, char test_chr[32])
{
    strcpy(_aplex_tty->tty_node, tty_node);
    _aplex_tty->stop_count = stop_count;
    _aplex_tty->usleep_time = usleep_time;
    strcpy(_aplex_tty->test_chr, test_chr);
}

void test_tty(struct aplex_tty *_aplex_tty)
{
    int fd, count = 0, true_num = 0 , nset, read_val, strcasecmp_num;
    char read_buf[256];

    fd = open(_aplex_tty->tty_node, O_RDWR);
    if(fd < 0)
        goto TEST_ERR;

    nset = set_opt(fd, 115200, 8, 'N', 1);
    if (nset == -1)
        goto TEST_ERR1;

    while(1)
    {
        write(fd, _aplex_tty->test_chr, sizeof(_aplex_tty->test_chr));
        count ++;

        memset(read_buf, 0, sizeof(read_buf));

        read_val = read(fd, read_buf, sizeof(read_buf));
        if(read_val > 0)
        {
            strcasecmp_num = strcasecmp(read_buf, _aplex_tty->test_chr);
            if (strcasecmp_num == 0)
                true_num++;
        }

        if ((count % 500) == 0)
        {
            printf("%s  loss num    :    %d         \n", _aplex_tty->tty_node, (count-true_num));
            printf("%s  count num   :    %d         \n", _aplex_tty->tty_node, count);
            printf("\n\n");
        }
        if (count > _aplex_tty->stop_count)
            break;

        usleep(_aplex_tty->usleep_time);
    }

    close(fd);
    return;

TEST_ERR1:
    close(fd);
TEST_ERR:
    printf("open %s  error\n", _aplex_tty->tty_node);
    return;
}
