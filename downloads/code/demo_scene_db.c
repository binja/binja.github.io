/*
    gcc demo_scene_db.c -o demo_scene_db -Wl,-z,relro,-z,now -Wl,-s -fstack-protector-all
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>

#define MAX_NUM_ANIME 512

typedef char Html[0x100];

struct Animation {
    char description[0xf0];
    uint64_t size_scenes;
    Html scenes[];
};

struct SampleAnimation {
    char description[0xf0];
    uint64_t size_scenes;
    Html scenes[3];
};

#define LUMP_BIT (1)
#define IS_LUMP(anime) ((anime)->size_scenes & LUMP_BIT)
#define GET_SIZE(anime) ((anime)->size_scenes & ~LUMP_BIT)

int fd;
uint64_t cmd;
uint64_t num_animations;
struct Animation *animations[MAX_NUM_ANIME];

void _edit_scenes(struct Animation *anime);

size_t read_fd(char *buf, size_t buf_size, char splitter) {
    size_t i = 0;

    if (buf_size == 0) return 0;

    while (i < buf_size - 1) {
        if (read(fd, buf+i, 1) != 1 || buf[i] == splitter || buf[i] == '\0') {
            buf[i] = '\0';
            break;
        }

        ++i;
    }

    if (i == buf_size - 1) buf[i] = '\0';
    return i;
}

void send_fd(char *buf, size_t buf_size) {
    send(fd, buf, buf_size, 0);
}

#define myputs(s) send_fd(s "\n", sizeof(s "\n"))

uint64_t get_integer() {
    uint64_t ret = 0;
    char buf[16];
    read_fd(buf, sizeof(buf), '\n');
    sscanf(buf, "%" SCNu64, &ret);
    return ret;
}

void print_banner(void) {
    myputs("                                                                                                        ");
    myputs(" ██████╗ ███████╗███╗   ███╗ ██████╗     ███████╗ ██████╗███████╗███╗   ██╗███████╗    ██████╗ ██████╗  ");
    myputs(" ██╔══██╗██╔════╝████╗ ████║██╔═══██╗    ██╔════╝██╔════╝██╔════╝████╗  ██║██╔════╝    ██╔══██╗██╔══██╗ ");
    myputs(" ██║  ██║█████╗  ██╔████╔██║██║   ██║    ███████╗██║     █████╗  ██╔██╗ ██║█████╗      ██║  ██║██████╔╝ ");
    myputs(" ██║  ██║██╔══╝  ██║╚██╔╝██║██║   ██║    ╚════██║██║     ██╔══╝  ██║╚██╗██║██╔══╝      ██║  ██║██╔══██╗ ");
    myputs(" ██████╔╝███████╗██║ ╚═╝ ██║╚██████╔╝    ███████║╚██████╗███████╗██║ ╚████║███████╗    ██████╔╝██████╔╝ ");
    myputs(" ╚═════╝ ╚══════╝╚═╝     ╚═╝ ╚═════╝     ╚══════╝ ╚═════╝╚══════╝╚═╝  ╚═══╝╚══════╝    ╚═════╝ ╚═════╝  ");
    myputs("                                                                                                        \n");
}

void print_menu(void) {
    myputs("========== Menu =========");
    myputs("1: create animation");
    myputs("2: edit description");
    myputs("3: edit scenes");
    myputs("4: show animations");
    myputs("5: copy animation");
    myputs("6: combine two animations");
    myputs("7: exit");
    myputs("=========================\n");
}

void create_animation(void) {
    char res[3];
    uint64_t num;

    if (num_animations >= MAX_NUM_ANIME) {
        myputs("You have enough animations, don't you?");
        _exit(0);
    }

    dprintf(fd, "How many scenes does your animation use?: ");
    num = get_integer();
    if (num > 0x1000) {
        fprintf(stderr, "You wanna trouble me, ha?");
        _exit(0);
    }

    animations[num_animations] = calloc(1, sizeof(struct Animation) + sizeof(Html)*num);
    if (!animations[num_animations]) {
        fprintf(stderr, "This is unfortunate for you...");
        _exit(0);
    }

    animations[num_animations]->size_scenes = sizeof(Html)*num;
    dprintf(fd, "Do you want to use the region as one scene?[Y/n]: ");
    read_fd(res, sizeof(res), '\n');
    if (res[0] == 'Y' || res[0] == 'y') {
        animations[num_animations]->size_scenes |= LUMP_BIT;
    }

    read_fd(animations[num_animations]->description, 0xf0, '\n');
    _edit_scenes(animations[num_animations]);
    
    myputs("Your new animation has been created successfully.");
    ++num_animations;
}

void edit_description(void) {
    dprintf(fd, "Which description do you want to edit?: ");

    uint64_t idx = get_integer();
    if (idx >= num_animations) {
        fprintf(stderr, "No nip-slip.");
        _exit(0);
    }

    read_fd(animations[idx]->description, 0xf0, '\n');
    myputs("Done.");
}

void _edit_scenes(struct Animation *anime) {
    if (IS_LUMP(anime)) {
        read_fd((char*)anime->scenes, GET_SIZE(anime), '\n');
    } else {
        uint64_t i;
        uint64_t lim = anime->size_scenes/sizeof(Html);
        for (i=0; i<lim; i++) {
            read_fd(anime->scenes[i], sizeof(Html), '\n');
        }
    }
}

void edit_scenes(void) {
    dprintf(fd, "Which scenes do you want to edit?: ");

    uint64_t idx = get_integer();
    if (idx >= num_animations) {
        fprintf(stderr, "Shame on you!");
        _exit(0);
    }

    _edit_scenes(animations[idx]);
    myputs("Completed.");
}

void show_animations(void) {
    uint64_t i;

    dprintf(fd, "====== Animations =====");
    for (i=0; i<num_animations; i++) {
        dprintf(fd, "\n\n#%" PRIu64 ": \n", i);
        dprintf(fd, "Desc: %s", animations[i]->description);
        dprintf(fd, "\nScenes: ");
        if (IS_LUMP(animations[i])) {
            send_fd((char*)animations[i]->scenes, GET_SIZE(animations[i]));
        } else {
            uint64_t j;
            uint64_t lim = animations[i]->size_scenes/sizeof(Html);
            for (j=0; j<lim; j++) {
                send_fd(animations[i]->scenes[j], strlen(animations[i]->scenes[j]));
            }
        }
    }
    myputs("\n\n\n=======================\n");
}

void copy_animation(void) {
    uint64_t from, to;

    dprintf(fd, "Which animation do you want to copy?: ");
    from = get_integer();
    
    dprintf(fd, "Dest: ");
    to = get_integer();
    
    if (from >= num_animations || to >= num_animations || from == to) {
        fprintf(stderr, "I'm sorry. I can't hear you!");
        _exit(0);
    }

    uint64_t used_size;
    uint64_t size_from = animations[from]->size_scenes;
    uint64_t size_to = animations[to]->size_scenes;
    if (IS_LUMP(animations[from])) {
        if ((size_to & LUMP_BIT) == 0 && (size_from&~LUMP_BIT) > sizeof(Html)) {
            myputs("Watch out!");
            return;
        }

        memset(animations[to]->scenes, '\0', size_to & ~LUMP_BIT);

        used_size = size_from&~LUMP_BIT;
        if (size_to < used_size) {
            used_size = size_to;
        }

        memmove(animations[to]->scenes, animations[from]->scenes, used_size);
    } else {
        used_size = size_from;
        if (size_to < used_size) {
            used_size = size_to;
        }
        
        if (size_to & LUMP_BIT) {
            ((char*)animations[to]->scenes)[0] = '\0';
        }

        uint64_t i;
        uint64_t lim = used_size/sizeof(Html);
        for (i=0; i<lim; i++) {
            if (size_to & LUMP_BIT) {
                strcat((char*)animations[to]->scenes, animations[from]->scenes[i]);
            } else {
                strcpy(animations[to]->scenes[i], animations[from]->scenes[i]);
            }
        }
    }

    memmove(animations[to]->description, animations[from]->description, 0xf0);
}

void combine_animations(void) {
    uint64_t a, b;

    if (num_animations >= MAX_NUM_ANIME) {
        fprintf(stderr, "You have enough animations, don't you?");
        _exit(0);
    }

    dprintf(fd, "Which animations do you want to combine?: ");
    a = get_integer();
    b = get_integer();
    if (a >= num_animations || b >= num_animations || a == b) {
        fprintf(stderr, "Connection Refused :(");
        _exit(0);
    }

    if (IS_LUMP(animations[a]) || IS_LUMP(animations[b])) {
        uint64_t new_size = 0;
        
        if (IS_LUMP(animations[a])) {
            new_size += GET_SIZE(animations[a]);
            if (new_size < GET_SIZE(animations[a])) {
                fprintf(stderr, "pity");
                _exit(0);
            }
        } else {
            uint64_t i;
            for (i=0; i<animations[a]->size_scenes/sizeof(Html); i++) {
                uint64_t n = strlen(animations[a]->scenes[i]);
                new_size += n;
                if (new_size < n) {
                    fprintf(stderr, "pity");
                    _exit(0);
                }
            }
        }

        if (IS_LUMP(animations[b])) {
            new_size += GET_SIZE(animations[b]);
            if (new_size < GET_SIZE(animations[b])) {
                fprintf(stderr, "pity");
                _exit(0);
            }
        } else {
            uint64_t i;
            for (i=0; i<animations[b]->size_scenes/sizeof(Html); i++) {
                uint64_t n = strlen(animations[b]->scenes[i]);
                new_size += n;
                if (new_size < n) {
                    fprintf(stderr, "pity");
                    _exit(0);
                }
            }
        }

        new_size += 1;
        if (new_size < 1) {
            fprintf(stderr, "pity");
            _exit(0);
        }

        struct Animation *new_ref;
        if (GET_SIZE(animations[a]) >= new_size) {
            new_ref = animations[a];
        } else if (GET_SIZE(animations[b]) >= new_size) {
            new_ref = animations[b];
            uint64_t tmp = a;
            a = b;
            b = tmp;
        } else {
            uint64_t num = new_size/sizeof(Html) + (new_size%sizeof(Html) != 0);
            new_ref = calloc(1, sizeof(struct Animation) + sizeof(Html)*num);
            new_ref->size_scenes = sizeof(Html)*num;
            animations[num_animations] = new_ref;
            ++num_animations;
        }

        read_fd(new_ref->description, 0xf0, '\n');

        char *p = (char*)new_ref->scenes;
        if (IS_LUMP(animations[a])) {
            memmove(p, animations[a]->scenes, GET_SIZE(animations[a]));
            p += GET_SIZE(animations[a]);
        } else {
            uint64_t i;
            for (i=0; i<animations[a]->size_scenes/sizeof(Html); i++) {
                uint64_t n = strlen(animations[a]->scenes[i]);
                memmove(p, animations[a]->scenes[i], n);
                p += n;
            }
        }

        if (IS_LUMP(animations[b])) {
            memmove(p, animations[b]->scenes, GET_SIZE(animations[b]));
            p += GET_SIZE(animations[b]);
        } else {
            uint64_t i;
            for (i=0; i<animations[b]->size_scenes/sizeof(Html); i++) {
                uint64_t n = strlen(animations[b]->scenes[i]);
                memmove(p, animations[b]->scenes[i], n);
                p += n;
            }
        }

        new_ref->size_scenes |= LUMP_BIT;
        *p = '\0';
    } else {
        uint64_t new_size = animations[a]->size_scenes + animations[b]->size_scenes;
        if (new_size < animations[a]->size_scenes) {
            fprintf(stderr, "pity");
            _exit(0);
        }

        struct Animation *new_ref;
        if (animations[a]->size_scenes >= new_size) {
            new_ref = animations[a];
        } else if (animations[b]->size_scenes >= new_size) {
            new_ref = animations[b];
        } else {
            new_ref = calloc(1, sizeof(struct Animation) + new_size);
            new_ref->size_scenes = new_size;
            animations[num_animations] = new_ref;
            ++num_animations;
        }

        read_fd(new_ref->description, 0xf0, '\n');

        char *p = (char*)new_ref->scenes;
        memmove(p, animations[a]->scenes, animations[a]->size_scenes);
        p += animations[a]->size_scenes;
        memmove(p, animations[b]->scenes, animations[b]->size_scenes);
        new_ref->size_scenes &= ~LUMP_BIT;
    }

    myputs("LGTM");
}

void service(int csock) {
    struct SampleAnimation sample;

    fd = csock;
    dup2(fd, STDERR_FILENO);

    alarm(600);
    print_banner();

    num_animations = 1;
    animations[0] = (struct Animation*) &sample;
    strcpy(animations[0]->description, "A sample minidemo from http://www.p01.org/");
    animations[0]->size_scenes = sizeof(Html) * 3;
    strcpy(animations[0]->scenes[0], "<body onload=setInterval(\"for(t-=.1,x=h,c.height=300,Q=Math.cos;x--;)for(y=h;y--;c.getContext('2d').fillRect(x*4,y*4,N,N))for(N=D=4;X=D*x/h-D/2,Y=D*y/h-D/2,Z=D/2-9,D+=d=(X*X+Y*Y*Q(t/6+Q(D-X-Y))+Z*Z)/9-1+Q(X+t)*Q(Y-t),.1<d*N;)N-=.1\",t=h=75)><canvas id=c>");
    strcpy(animations[0]->scenes[1], "<body onload=E=c.getContext(\"2d\"),setInterval(F=\"t+=.2,Q=Math.cos;c.height=300;for(x=h;x--;)for(y=h;y--;E.fillRect(x*4,y*4,b-d?4:D/2,D/2))for(D=0;'.'<F[D*y/h-D/2|0?1:(d=t+D*Q(T=x/h-.5+Q(t)/8)&7)|(3.5+D*Q(T-8))<<3]&&D<8;b=d)D+=.1\",t=h=75)><canvas id=c>");
    strcpy(animations[0]->scenes[2], "<body onload=setInterval(F=\";t+=.1;Q=Math.cos;for(x=n=c.height=300;x-=4;)for(y=n;y-=4;d.fillRect(x,y,E,Z^_?4:E))for(D=0;(E=4-D/2)&&F<F[(t+D*Q(T=x/n-.5+Q(t/9))&7)*8|(Z=3.7+D*Q(T-8)&7)*4|(6.5-D*y/n-E)];_=Z)D+=1/8\",t=55),d=c.getContext('2d')><canvas id=c>");
    
    while (1) {
        print_menu();

        dprintf(fd, "> ");
        cmd = get_integer();

        if (cmd == 1) {
            create_animation();
        } else if (cmd == 2) {
            edit_description();
        } else if (cmd == 3) {
            edit_scenes();
        } else if (cmd == 4) {
            show_animations();
        } else if (cmd == 5) {
            copy_animation();
        } else if (cmd == 6) {
            combine_animations();
        } else if (cmd == 7) {
            break;
        } else {
            fprintf(stderr, "Undefined command: %" PRIu64 "\n", cmd);
        }
    }

    while (num_animations > 1) {
        free(animations[num_animations-1]);
        --num_animations;
    }
}

uint64_t cookie = 0;
void overwrite_cookie() {
    int fd;
    int ret;

    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) _exit(0);

    ret = read(fd, &cookie, sizeof(cookie) - 1);
    if (ret != sizeof(cookie) - 1) _exit(0);
    close(fd);

    asm volatile(
            ".intel_syntax noprefix\n"
            "mov rax, cookie\n"
            "mov fs:0x28, rax\n"
            "mov [rbp-0x8], rax\n"
            ".att_syntax noprefix\n"
            );

    cookie = 0; // erase from static mem
}

int main(void) {
    struct sockaddr_in addr;
    socklen_t addrlen;
    int server_sock;
    int client_sock;
    int opt = 1;

    signal(SIGCHLD, SIG_IGN);

    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_sock == -1) {
        _exit(-1);
    }

    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(11451);

    if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        _exit(-1);
    }

    listen(server_sock, 5);
    while (1) {
        addrlen = sizeof(addr);

        memset(&addr, 0, addrlen);
        client_sock = accept(server_sock, (struct sockaddr *)&addr, &addrlen);

        if (fork() == 0) {
            close(server_sock);

            if (setsid() != -1) {
                overwrite_cookie();
                service(client_sock);
                fclose(stderr);
            }

            shutdown(client_sock, SHUT_RDWR);
            close(client_sock);
            _exit(0);
        }

        close(client_sock);
    }
}

