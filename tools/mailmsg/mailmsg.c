static char *mailmsg_id = "@(#) $Id: mailmsg.c,v 1.2 2002/05/21 08:29:34 usr01475 Exp $";

/* 
 *  Project: IPMSG
 *  Author & Copyright:  KOHNO TAKASHI kouno@catnet.ne.jp
 *  Mail: kouno@catnet.ne.jp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAILMSG_VERSION "0.81"

#define IPMSG_VERSION		0x001
#define IPMSG_PORT			0x979
#define IPMSG_REVERSEICON	0x100

#define IPMSG_BR_ENTRY		0x00000001
#define IPMSG_BR_EXIT		0x00000002
#define IPMSG_ANSENTRY		0x00000003
#define IPMSG_BR_ISGETLIST	0x00000010
#define IPMSG_OKGETLIST		0x00000011
#define IPMSG_GETLIST		0x00000012
#define IPMSG_ANSLIST		0x00000013
#define IPMSG_SENDMSG		0x00000020
#define IPMSG_RECVMSG		0x00000021

#define IPMSG_SENDCHECKOPT	0x00000100
#define IPMSG_SECRETOPT		0x00000200
#define IPMSG_BROADCASTOPT	0x00000400
#define IPMSG_NOPOPUPOPT	0x00000800
#define IPMSG_PASSWORDOPT	0x00008000
#define IPMSG_NOLOGOPT 		0x00020000
#define IPMSG_NOADDLISTOPT  0x00080000

#define UDP_BUF			8192

#define GET_MODE( command )	( command & 0x000000ff )
#define GET_OPT( command )	( command & 0x0000ff00 )

#define FAIL	0
#define MAXLEN 0x1ff
#define MAXBUFLEN	0x01ff
#define MAXHEADDERLINE	0x00ff
#define MAXHEADDERLEN	0x01ff
#define MAXBODYLEN	0x01ff
#define MAXBODYLINE	0x00ff
#define ALIAS_LINEMAX (80*24)
#define REALNAMEMAX 0x100

#define DEFAULT_HEADDER "From:"
#define DEFAULT_MESSAGE "メールが届きました。"
#define NOTEXIST "Not Exist the Headder"
#define BODY_START "--- ここより本文 ---\n"
#define ENV_LOGNAME "LOGNAME"
#define EDITOR_ENV_NAME "EDITOR"
#define EDITOR "vi"
#define SENDFILE_ENV_NAME "MAILMSGSFILE"
#define SENDFILE "/tmp/mailmsg.send"
#define ENV_HOME "HOME"
#define TIPMSGDIR "tipmsg"
#define DEFAULT_USERLIST "userlist"
#define DEFAULT_ALIASES "aliases"
#define ALIAS_DELIM ", \n\r"
#define DEFAULT_RECENTFILE "recent_recvfrom"

// ソケット作成失敗時のリトライ回数とリトライ区間のウェイト時間(sec)
#define MAX_RETRY  10
#define RETRY_WAIT 1

struct userlist {
    struct userlist* prev;
    struct userlist* next;
    char buf[PATH_MAX];
};

struct aliases {
    struct aliases* prev;
    struct aliases* next;
    int num;
    char alias[PATH_MAX];
    char* real[REALNAMEMAX];
};

void make_msg(char *buf, int command, char *msg);
void get_headder(char **headder, char *body);
char *get_target(char *msg, const char *target, char **headder);
char *get_real(char *str);
void get_all(char *msg);
void print_usage(void);
void dummy_function(char *dummy,...);
unsigned long  GetHostAddr(char* name);
int CreateUserlist(char* file);
void DestoryUserlist(void);
int AppendSendList(char* name);
int CreateAliasUserlist(char* name, struct userlist** p_first, struct userlist** p_last);

#if defined(DEBUG)
#define line fprintf(stderr, "Line %d\n", __LINE__)
#define d_printf printf
#else
#define debug
#define d_printf dummy_function
#endif

int HeadderLine = 0;
int ipmsg_port;
int body_request = 0;
int body_line = 0;
int direct_msg = 0;
char *g_editor = NULL;
char *g_sendfile = NULL;
char g_entry_userlist[PATH_MAX] = "";
char g_aliases[PATH_MAX] = "";
char g_recent_recvfrom[PATH_MAX] = "";
struct userlist* g_entry_userlist_first = NULL;
struct aliases* g_aliases_first = NULL;
struct userlist* g_send_userlist_first = NULL;

int main(int argc, char *argv[])
{
    int sd, lp;
    struct sockaddr_in saddr;
//    struct hostent *host;
    char *buf, *work, *sendbuf, *msg;
    long addr;
    char **headder;
    char *body;
    int opt_flag = IPMSG_NOLOGOPT | IPMSG_NOADDLISTOPT;
    int nBufLen;
    int nBodyLen;
    char *ptr;
    char *sendhost;
    int cpid;
    int statusp;
//    int num=0;
//    int i_tmp;
    struct userlist* user;
    char ans[3];
    char to_file[PATH_MAX];
    FILE* fp;
	int error_count = 0;

    ipmsg_port = IPMSG_PORT;

    /* 引数の解析 - ハイフン1つにつき1引数のバージョン */
    for (; argc > 1 && argv[1][0] == '-'; argv++, argc--) {
        switch (argv[1][1]) {
        case 'l':
        case 'L':
            opt_flag &= ~IPMSG_NOLOGOPT;
            break;
        case '?':
        case 'h':
        case 'H':
            print_usage();
            return 0;
        case 'p':
        case 'P':
            ipmsg_port = strtol(&argv[1][2], &ptr, 0);
            d_printf("PORT %d(%s)\n", ipmsg_port, &argv[1][0]);
            if (ipmsg_port == 0)
                ipmsg_port = IPMSG_PORT;
            break;
        case 'b':
        case 'B':
            body_request = strtol(&argv[1][2], &ptr, 0);
            d_printf("BODY %d(%s) LINE PRINT\n", body_request, &argv[1][0]);
            if (body_request > MAXBODYLINE) {
                body_request = MAXBODYLINE;
            }
            break;
        case 's':
        case 'S':
            d_printf("Enable Secret option.\n");
            opt_flag |= IPMSG_SECRETOPT;
            break;
        case 'w':
        case 'W':
            d_printf("Enable Password option.\n");
            opt_flag |= IPMSG_PASSWORDOPT | IPMSG_SECRETOPT;
            break;
        case 'D':
            opt_flag &= ~IPMSG_NOADDLISTOPT;
        case 'd':
            d_printf("Enable Direct send.\n");
            direct_msg = 1;
            opt_flag &= ~IPMSG_NOLOGOPT;
            break;
        case 'c':
        case 'C':
            d_printf("Enable Send check option.\n");
            opt_flag |= IPMSG_SENDCHECKOPT;
            break;
        case 'e':
        case 'E':
            if (argv[1][2] != '\0') {
                d_printf("Using editor(%s) designated argument.\n", g_editor);
                g_editor = &argv[1][2];
            } else if (!(g_editor = getenv(EDITOR_ENV_NAME))) {
                g_editor = EDITOR;
                d_printf("Using default editor(%s).\n", g_editor);
            } else {
                d_printf("Using editor(%s) designated enviroment .\n", g_editor);
            }
            break;
        case 'f':
        case 'F':
            if (argv[1][2] != '\0') {
                g_sendfile = &argv[1][2];
            } else if (!(g_sendfile = getenv(SENDFILE_ENV_NAME))) {
                g_sendfile = SENDFILE;
            }
            d_printf("Using file(%s) for send message.\n", g_sendfile);
            break;
        case 'u':
        case 'U':
            if (argv[1][2] != '\0') {
                strcpy(g_entry_userlist, &argv[1][2]);
            } else {
                sprintf(g_entry_userlist, "%s/%s", getenv(ENV_HOME), TIPMSGDIR);
                sprintf(g_entry_userlist, "%s/%s/%s", getenv(ENV_HOME), TIPMSGDIR, DEFAULT_USERLIST);
            }
            d_printf("Using userlist file(%s).\n", g_entry_userlist);
            CreateUserlist(g_entry_userlist);
            break;
        case 'a':
        case 'A':
            if (argv[1][2] != '\0') {
                strcpy(g_aliases, &argv[1][2]);
            } else {
                sprintf(g_aliases, "%s/%s", getenv(ENV_HOME), TIPMSGDIR);
                sprintf(g_aliases, "%s/%s/%s", getenv(ENV_HOME), TIPMSGDIR, DEFAULT_ALIASES);
            }
            d_printf("Using alias file(%s).\n", g_aliases);
            CreateAliases(g_aliases);
            break;
        case 'r':
        case 'R':
            if (argv[1][2] != '\0') {
                strcpy(to_file, &argv[1][2]);
            } else {
                sprintf(to_file, "%s/%s/%s", 
                        getenv(ENV_HOME), TIPMSGDIR, DEFAULT_RECENTFILE);
            }
            if (( fp = fopen(to_file, "r") ) != NULL ){
                fgets(g_recent_recvfrom, PATH_MAX, fp);
                fclose(fp);
                if (argv[1][1] == 'R'){
                    fprintf(stdout, 
                            "Do you send messegae to [%s]? (Yes/No) : ", 
                            g_recent_recvfrom);
                    fgets(ans, 3, stdin);
                    if (toupper(ans[0]) != 'Y'){
                        fprintf(stdout, "Cancel!\n");
                        return 1;
                    }
                }
                d_printf("Using RECENTRECVFROM(%s) .\n", g_recent_recvfrom);
            } else {
                fprintf(stderr, "Can't open recent file.(%s)\n", to_file);
                return 1;
            }
            break;
        default:
            break;
        }
    }

    /* 引数の不足を警告 */
    if (!g_recent_recvfrom[0] && argc < 2) {
        fprintf(stderr, "NOT ENOUGH PARAMETER MUST NEED 1 -> mailmsg hostname\n");
        goto ERR;
    }

    /* -e 指定時のファイルエディット */
    if (g_editor) {
        if (!g_sendfile) {
            if (!(g_sendfile = getenv(SENDFILE_ENV_NAME))) {
                g_sendfile = SENDFILE;
            }
        }
        if ((cpid = fork()) == 0) {
            char command[PATH_MAX];
            sprintf(command, "%s %s", g_editor, g_sendfile);
            execlp("/bin/sh", "sh", "-c", command, NULL);
            /* execlp が成功すればこれ以降は実行されない */
            perror("Can't exec editor");
            goto ERR;
        }
        if (cpid == -1) {
            perror("Can't create process");
            goto ERR;
        }
        wait(&statusp);
        printf("Send message OK?[y/n]\n");
        if (getchar() != 'y') {
            printf("Not send\n");
            goto ERR;
        }
    }

    /* ボディ部メモリアロケート */
    if (body_request) {
        if (!(body = malloc(UDP_BUF))) {
            perror("Body Buffer Allocate Error in MailMsg");
            goto ERR;
        }
    } else {
        body = NULL;
    }

    /* メッセージ部メモリアロケート */
    if ((msg = malloc(UDP_BUF)) == NULL) {
        perror("MSG Buffer Allocate Error\n");
        goto ERR;
    }
    msg[0] = '\0';

    /* テンポラリバッファメモリアロケート */
    if ((buf = malloc(UDP_BUF)) == NULL) {
        perror("Buffer Allocate Error in MailMsg");
        goto ERR;
    }
    /* 送信バッファメモリアロケート */
    if ((sendbuf = malloc(UDP_BUF)) == NULL) {
        perror("Send Buffer Allocate Error in MailMsg");
        goto ERR;
    }
    if ((work = malloc(UDP_BUF)) == NULL) {
        perror("Work Buffer Allocate Error in MailMsg");
        goto ERR;
    }
    /* ヘッダ部バッファメモリアロケート */
    if ((headder = malloc(sizeof(char *) * MAXHEADDERLINE)) == NULL) {
        perror("Headder Buffer Allocate Error in MailMsg");
        goto ERR;
    }
    for (lp = 0; lp < MAXHEADDERLINE; lp++) {
        if ((headder[lp] = malloc(MAXHEADDERLEN)) == NULL) {
            fprintf(stderr, "Headder(%d) Buffer Allocate Error in MailMsg", lp);
            perror(NULL);
            goto ERR;
        }
    }

    /* ヘッダ部と指定量ボディ部の取得 */
    if (direct_msg) {
        get_all(msg);
        d_printf("Go to SEND.\n");
        goto SEND;
    } else {
        get_headder(headder, body);
    }

    d_printf("Mail Message Mode.\n");
    if (argc == 2 && !direct_msg) {
        get_target(buf, DEFAULT_HEADDER, headder);
        sprintf(msg, "%s\n\n        から %s\n", buf, DEFAULT_MESSAGE);
    }

    /* 指定ヘッダの抜きだし */
    sendhost = argv[1];
    for (; argc > 2 && !direct_msg; argc--,argv++) {
        char tmp[MAXHEADDERLEN];
        get_target(tmp, argv[2], headder);
        if (strcmp(tmp, NOTEXIST) != 0) { /* 指定ヘッダは存在するか？ */
            if (tmp) {
                sprintf(work, "%s\n  %s", argv[2], tmp); /* */
                sprintf(buf, "%s%s\n", buf, work);
            }
        }
    }
    sprintf(msg, "%s\n\n%s\n", DEFAULT_MESSAGE, buf);
    argv[1] = sendhost;

    /* ボディ部の接続 */
    if (body_request) {
        nBufLen = strlen(msg);
        nBodyLen = strlen(body);
        if (nBufLen + nBodyLen >= UDP_BUF) {
            if (nBufLen <= UDP_BUF) {
                body[UDP_BUF - nBufLen] = '\0';
            }
        }
        strcat(msg, body);
    }
    msg[UDP_BUF - 100] = '\0';

 SEND:
    /* 複数人に対する送信 */
    for (; argc > 1; argc--,argv++) {
        AppendSendList(argv[1]);
    }
    /* RECENTRECVFROM ユーザーの追加*/
    if (g_recent_recvfrom[0]){
        AppendSendList(g_recent_recvfrom);
    }

	for (user = g_send_userlist_first; user; user = user->next) {
		d_printf("Send (%s)[%08x]\n", user->buf, user->next);
		if (!(addr = GetHostAddr(user->buf))){
			continue;
		}
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(ipmsg_port);
		saddr.sin_addr.s_addr = addr;

		/* 送信バイトの作成 */
		make_msg(sendbuf, IPMSG_SENDMSG | opt_flag, msg);

		/* ソケットの作成と準備 */
		for (lp = 0; lp < MAX_RETRY; lp++) {
			if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) != -1) {
				break;
			}
			sleep(RETRY_WAIT);
		}
		if (sd == -1) {
			perror("Can't create socket");
			goto ERR;
		}

		/* 送信 */
		if (sendto(sd, sendbuf, strlen(sendbuf) + 1, 0,
				   (struct sockaddr *) &saddr, sizeof(saddr)) == -1) {
			d_printf( "Error to %s\n", user->buf );
			error_count ++;
			/*fprintf(stderr, "Sendto Err(%s) : ", argv[1]);*/
			/*perror(NULL);*/
		}
		close( sd );
	}

    DestoryUserlist();
	if( error_count > 0 ){
		return error_count;
	}
    return 0;

 ERR:
    DestoryUserlist();
    return 1;
}

/* 送出メッセージ作成ルーチン */
void make_msg(char *buf, int command, char *msg)
{
    static long packet_no = 0;
    char username[MAXBUFLEN];
    char hostname[MAXBUFLEN];
    char* p;

    if (gethostname(hostname, sizeof(hostname)) == -1) {
        strcpy(hostname, "Unknow host");
    }
    if (( p = strchr(hostname, '.') ) != NULL ){
        *p = '\0';
    }
    
    if (direct_msg) {
        strcpy(username, getpwuid(getuid())->pw_name);
        if (!username[0]){
            strcpy(username, "Unknown user");
        }
#if(0)        
        if (!getenv(ENV_LOGNAME)) {
            strcpy(username, "Unknown user");
        } else {
            strcpy(username, getenv(ENV_LOGNAME));
        }
#endif
    }
    if (!packet_no){
        packet_no = time(&packet_no) + ((long) getpid() << 16);
    } else {
        packet_no++;
    }

    if (!direct_msg) {
        sprintf(buf, "%d:%d:%s:%s:%d:%s", IPMSG_VERSION, packet_no, "MailMsg", hostname, command, msg ? msg : "");
    } else {
        sprintf(buf, "%d:%d:%s:%s:%d:%s", IPMSG_VERSION, packet_no, username, hostname, command, msg ? msg : "");
    }
}

/* ヘッダ抽出ルーチン */
void get_headder(char **headder, char *body)
{
    char buf[MAXBODYLEN];
    int buflen, bodylen;
    int lp;
    FILE *fp = stdin;

    /* 読み込みファイルは指定? */
    if (g_sendfile) {
        if (!(fp = fopen(g_sendfile, "r"))) {
            fp = stdin;
        }
    }
    /* ヘッダ部抽出 */
    while (fgets(headder[HeadderLine++], MAXHEADDERLEN, fp)) {
        /* 1行の長さチェック */
        if (strlen(headder[HeadderLine - 1]) >= MAXHEADDERLEN - 1) {
            headder[HeadderLine - 1][MAXHEADDERLEN - 1] = '\0';
            while (getchar() != '\n');
        }
        if (headder[HeadderLine - 1][0] == '\n') {
            break;
        }
        /* 行数のチェック */
        if (HeadderLine > MAXHEADDERLINE) {
            while (fgets(headder[HeadderLine], MAXHEADDERLEN, fp)){
                if (headder[HeadderLine - 1][0] == '\n') {
                    break;
                }
            }
            break;
        }
    }
    /* body部 抽出ルーティン */
    if (body) {
        strcpy(body, BODY_START);
        for (lp = 0; fgets(buf, MAXBODYLEN, fp) && lp < body_request; lp++) {
            buflen = strlen(buf);
            bodylen = strlen(body);
            if (buflen + bodylen >= UDP_BUF) {
                break;
            }
            strcat(body, buf);
        }
    }
    /* pipe 終了 */
    while (fgets(buf, MAXBODYLEN, fp));
    if (fp != stdin) {
        fclose(fp);
    }
}

/* ヘッダ情報抽出ルーチン */
char *
get_target(char *ret, const char *target, char **headder)
{
    char str[MAXHEADDERLEN];
    char field_name[MAXBUFLEN], *field, field2[MAXHEADDERLEN];
    int flag, len, lp, ln = 0, lp2 = 0;

    while (ln < HeadderLine) {
        /* 1行得る */
        strncpy(str, headder[ln++], MAXHEADDERLEN);

        /* ':'が含まれていなければ次の行へ */
        if ((field = strchr(str, ':')) == NULL) {
            continue;
        }
        /* 次の行の先頭が SPACE,TABであれば次の行を追加する */
        while (headder[ln][0] == '\t' ||
               headder[ln][0] == ' ') {
            if ((int) (strlen(str) + strlen(headder[ln])) > MAXHEADDERLEN)
                break;
            strcat(str, headder[ln++]);
        }

        field++;                /* filedの値を ':'以降とする */
        len = strlen(field);

        /* ヘッダが複数行の場合、不要なTAB、改行、Spaceを削除する */
	    flag = 0;	
		lp2 = 0;
        for (lp = 0; lp < len; lp++) {
            if (field[lp] == '\t' ||
                field[lp] == '\r' ||
                field[lp] == '\n') {
                flag = 1; 
				continue;
            }else
			{
			 if (field[lp] == ' ' && flag == 1){
			 continue;
			 }else{
			 field2[lp2] = field[lp];
			 lp2++;
			 flag = 0;
			 }}
        }
		field2[lp2] = '\0';

        /* ヘッダ項目の認識 */
        strncpy(field_name, str, field - str);
        field_name[field - str] = '\0';

        /* ヘッダ項目がtargetと等しければその項目の内容を返す */
        if (strcmp(field_name, target) == 0) {
            if (strcpy(ret, field2) == NULL) {
                strcpy(ret, "Unknown because strcpy error");
            }
            break;
        }
    }

    /* 指定ターゲットが存在しない場合 */
    if (ln >= HeadderLine) {
        strcpy(ret, NOTEXIST);
    }
#if !defined(MIME)
    get_real(ret);
#endif
    return ret;
}

char *
get_real(char *str)
{
    char *ret = str, *work;
    if ((ret = strchr(str, '<')) != NULL) {
        work = strchr(++ret, '>');
        *work = '\0';
        strcpy(str, ret);
#if(1)
    } else {
        if ((work = strchr(str, '(')) != NULL) {
            *work = '\0';
            ret = str;
        }
#endif
    }
    return ret;
}

/* 全文抽出 */
void get_all(char *msg)
{
    char buf[MAXLEN];
    FILE *fp = stdin;

    if (g_sendfile) {
        if (!(fp = fopen(g_sendfile, "r"))) {
            fp = stdin;
        }
    }
    while (fgets(buf, MAXLEN - 1, fp)) {
        buf[MAXLEN - 1] = '\0';
        if (strlen(buf) + strlen(msg) > UDP_BUF - 100) {
            break;
        }
        strcat(msg, buf);
    }

    if (fp != stdin) {
        fclose(fp);
    }
}

unsigned long GetHostAddr(char* name)
{
    unsigned long addr;
    struct userlist* ul;
    struct hostent *host;
    char *ptr = NULL;
    char* tname = name; /* Target name */
    char c;

    for (ul = g_entry_userlist_first; ul; ul=ul->next){
		char buf[ 128 ], *p;
		strcpy( buf, ul->buf );
		if( ( p = strrchr( buf, '[' ) ) != NULL ){
			*p = '\0';
		}
        if ( strcmp( buf, name ) == 0 ){
            d_printf("HIT %s\n", ul->buf);
            tname = ul->buf;
            break;
        }
    }

    if (( ptr = strrchr(tname, '[') ) != NULL ) {
		tname = ptr + 1;
		if ( ( ptr = strrchr(tname, ']') ) != NULL ) {
			c = *ptr;
			*ptr = '\0';
		}
		d_printf( "host = %s\n", tname );
		if ((addr = inet_addr(tname)) == -1) {
			if ((host = (struct hostent *)gethostbyname(tname)) != NULL){
				addr = *(long *) host->h_addr;
			} else {
				fprintf(stderr, "invalid hostname(%s) : ", tname);
				perror(NULL);
				if (ptr){
					*ptr = c;
				}
				return 0;
			}
		}
	} else {
		if ((host = (struct hostent *)gethostbyname(tname)) != NULL) {
			addr = *(long *) host->h_addr;
		}
	}
    if (ptr){
        *ptr = c;
    }
    return addr;
}

int CreateUserlist(char* file)
{
    FILE* fp;
    struct userlist* ul;
    struct userlist* pul = g_entry_userlist_first;
    char buf[PATH_MAX];

    if (!(fp = fopen(file, "r"))){
        fprintf(stderr, "Can't open userlist file(%s). : ", file);
        perror(NULL);
        return 0;
    }

    while(fgets(buf, PATH_MAX, fp)){

		if( buf[ 0 ] == '\0' || buf[ 0 ] == '#' )continue;

        if (!(ul = (struct userlist*)malloc(sizeof(struct userlist)))){
            perror("Can't allocate memory.");
            return 0;
        }

        d_printf("Add entry user list - %s", buf);
        strcpy(ul->buf, buf);
        ul->next = NULL;
        ul->prev = pul;

        if (!pul){
            pul = g_entry_userlist_first = ul;
        } else {
            pul->next = ul;
        }
        pul = ul;
    }
    return 1;
}

void DestoryUserlist(void)
{
    struct userlist* ul;
    struct userlist* nul;

    ul = g_entry_userlist_first;
    while(ul){
        nul = ul->next;
        free(ul);
        ul = nul;
    }
}

int CreateAliases(char* file)
{
    FILE* fp;
    struct aliases* al;
    struct aliases* pal = g_aliases_first;
    char buf[ALIAS_LINEMAX];
    char* pa; /* Pointer to Alias end */
    char* pr; /* Pointer to real name */
    int num = 0;
    
    if (!(fp = fopen(file, "r"))){
        fprintf(stderr, "Can't open alias file(%s). : ", file);
        perror(NULL);
        return 0;
    }
    
    while(fgets(buf, ALIAS_LINEMAX, fp)){
        if (buf[0] == '#'|| buf[0] == ' ' || buf[0] == '\t'){
            continue;
        }
        
        if (!(pa = strchr(buf, ' ')) && !(pa = strchr(buf, '\t'))){
            continue;
        }
        *pa = '\0';
        pa++;
        
        for (; *pa == ' ' || *pa == '\t'; pa++);
        if (*pa == '\n' || *pa == '\r' || *pa == '\0'){
            continue;
        }
        
        if (!(al = (struct aliases*)malloc(sizeof(struct aliases)))){
            perror("Can't allocate memory.");
            return 0;
        }
        
        strcpy(al->alias, buf);
        d_printf("Alias is [%s]\n", al->alias);
        
        for (num = 0, pr = strtok(pa, ALIAS_DELIM); pr && num < REALNAMEMAX-1; pr = strtok(NULL, ALIAS_DELIM), num++){
            if (!(al->real[num] = (char*)malloc(strlen(pr)+1))){
                perror("Can't allocate memory.");
                return 0;
            }
            strcpy(al->real[num], pr);
            d_printf("Realname[%d] is [%s]\n", num, al->real[num]);
        }
        al->real[num] = NULL;
        al->num = num;
        
        al->next = NULL;
        al->prev = pal;
        if (!pal){
            pal = g_aliases_first = al;
        } else {
            pal->next = al;
        }
        pal = al;
        
    }
    return 1;
}

void DestoryAliases(void)
{
    struct aliases* al;
    struct aliases* nal;
    int num;

    al = g_aliases_first;
    while(al){
        nal = al->next;
        for (num=0; al->real[num]; num++){
            free(al->real[num]);
        }
        free(al);
        al = nal;
    }
}

/* 送信先ユーザーリストにユーザーを追加する
   エイリアスが設定されていればエイリアスの展開も行なう。*/
int AppendSendList(char* name)
{
    static struct userlist* last_user = NULL;
    struct userlist* first = NULL;
    struct userlist* last = NULL;

    if (!name){
        fprintf(stderr, "No refference argument in AppendSendList.");
        return 0;
    }

    d_printf("Append send list(%s)\n", name);

    if (g_aliases_first){
        CreateAliasUserlist(name, &first, &last);
    }

    if (!g_aliases_first || !first) {
        if (!(first = (struct userlist*)malloc(sizeof(struct userlist)))){
            perror("Can't allocate memory.");
            return 0;
        }
        strcpy(first->buf, name);
        last = first;
    }

    first->prev = last_user;
    last->next = NULL;

    if (last_user){
        last_user->next = first;
        last_user = last;
    } else {
        last_user = last;
        g_send_userlist_first = first;
    }

    return 1;
}

/* エイリアス展開を行なう */
int CreateAliasUserlist(char* name, struct userlist** p_first, struct userlist** p_last)
{
    struct aliases* at;
    struct userlist* user = NULL;
    int lp;

    *p_last = *p_first = NULL;
    for (at = g_aliases_first; at; at = at->next){
        if (strcmp(at->alias, name) == 0){
            for (lp=0; at->real[lp]; lp++){
                d_printf("Append alias user list(%d:%s)\n", lp, at->real[lp]);
                if (!(user = (struct userlist*)malloc(sizeof(struct userlist)))){
                    perror("Can't allocate memory.");
                    return 0;
                }
                strcpy(user->buf, at->real[lp]);
                user->prev = *p_last;
                user->next = NULL;
                if (*p_last){
                    (*p_last)->next = user;
                    *p_last = user;
                } else {
                    *p_last = *p_first = user;
                }
            }
            break;
        }
    }
    if (!at){
        d_printf("No match alias(%s).", name);
    }
    return 1;
}

/* 非デバッグ環境用のダミー関数 */
void dummy_function(char *dummy,...)
{
}

void print_usage(void)
{
    printf("MailMsg(biff tool) Version %s\n", MAILMSG_VERSION);
    printf("Usage:\n");
    printf("mailmsg [-option] Hostname DisplayHeadder:\n");
    printf("Option:\n");
    printf("\t-l IPMSG loging option\n");
    printf("\t-d Command line IP Messenger sender(Direct mode)\n");
    printf("\t-s Secret operate option\n");
    printf("\t-w Secret & Lock operate option\n");
    printf("\t-c Send Check operate option(Can't recive MailMsg)\n");
    printf("\t-p Send port\n");
    printf("\t-b Send body line\n");
    printf("\t-e Edit file before send(Not use stdin)\n");
    printf("\t-f Send message in file(Not use stdin)\n");
    printf("\t-u Using user list.(DEFAULT $HOME/tipmsg/userlist)\n");
    printf("\t-a Using alias list.(DEFAULT $HOME/tipmsg/aliases)\n");
    printf("\t-h Print this usage(? is same)\n");
    printf("Example1: ~/.forward file\n");
    printf("--- begin ---\n");
    printf("\"| nkf -m -s | mailmsg -p2000 -b50 hogehoge From: Subject:\"\n");
    printf("--- end ---\n");
    printf("Example2: Direct send with userlist & aliases.\n");
    printf("\t# nkf -su | mailmsg -p2000 -a -u -d person:mashine 3rd-group\n");
    printf("\n");
}
