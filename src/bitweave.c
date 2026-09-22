#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define BW_VERSION "1.4.0"

typedef struct { uint8_t *p; size_t n, cap; } Buf;
static _Noreturn void die(const char *m){ fprintf(stderr,"bitweave: %s\n",m); exit(2); }
static _Noreturn void die2(const char *m,const char *x){ fprintf(stderr,"bitweave: %s: %s\n",m,x); exit(2); }
static void grow(Buf *b,size_t add){ if(b->n+add<=b->cap)return; size_t c=b->cap?b->cap:256; while(c<b->n+add)c*=2; uint8_t *p=(uint8_t*)realloc(b->p,c); if(!p)die("out of memory"); b->p=p;b->cap=c; }
static void put8(Buf*b,uint8_t v){grow(b,1);b->p[b->n++]=v;}
static void put32(Buf*b,uint32_t v){for(int i=0;i<4;i++)put8(b,(uint8_t)(v>>(8*i)));}
static void put64(Buf*b,uint64_t v){for(int i=0;i<8;i++)put8(b,(uint8_t)(v>>(8*i)));}
static void patch16(Buf*b,size_t o,uint16_t v){b->p[o]=v&255;b->p[o+1]=(v>>8)&255;}
static void patch32(Buf*b,size_t o,uint32_t v){for(int i=0;i<4;i++)b->p[o+i]=(uint8_t)(v>>(8*i));}
static void patch64(Buf*b,size_t o,uint64_t v){for(int i=0;i<8;i++)b->p[o+i]=(uint8_t)(v>>(8*i));}
static Buf read_file(const char *path){
 int owned=strcmp(path,"-")!=0;
 int fd=owned?open(path,O_RDONLY):STDIN_FILENO;
 if(fd<0)die2("cannot open",path);
 Buf b={0}; uint8_t t[8192];
 for(;;){ssize_t nr=read(fd,t,sizeof t);if(nr<0){if(owned)(void)close(fd);die2("read failed",path);}if(nr==0)break;grow(&b,(size_t)nr);if(!b.p){if(owned)(void)close(fd);die("internal allocation failure");}memcpy(b.p+b.n,t,(size_t)nr);b.n+=(size_t)nr;}
 if(owned)(void)close(fd);
 return b;
}
static void write_file(const char *path,const void*p,size_t n){
 int owned=strcmp(path,"-")!=0;
 int fd=owned?open(path,O_WRONLY|O_CREAT|O_TRUNC,0666):STDOUT_FILENO;
 if(fd<0)die2("cannot create",path);
 const uint8_t *q=(const uint8_t*)p; size_t done=0;
 while(done<n){ssize_t nw=write(fd,q+done,n-done);if(nw<=0){if(owned)(void)close(fd);die2("write failed",path);}done+=(size_t)nw;}
 if(owned)(void)close(fd);
}

/* Small self-contained SHA-256 for provenance manifests. */
typedef struct{uint32_t h[8];uint64_t bits;uint8_t b[64];size_t n;} SHA;
static uint32_t rr(uint32_t x,unsigned n){return (x>>n)|(x<<(32-n));}
static void sha_block(SHA*s,const uint8_t*p){
 static const uint32_t k[64]={
 0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
 0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
 0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
 0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
 0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
 0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
 0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
 0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};
 uint32_t w[64]={0}; for(int i=0;i<16;i++)w[i]=((uint32_t)p[i*4]<<24)|((uint32_t)p[i*4+1]<<16)|((uint32_t)p[i*4+2]<<8)|p[i*4+3];
 for(int i=16;i<64;i++){uint32_t x=w[i-15],y=w[i-2];uint32_t s0=rr(x,7)^rr(x,18)^(x>>3),s1=rr(y,17)^rr(y,19)^(y>>10);w[i]=w[i-16]+s0+w[i-7]+s1;}
 uint32_t a=s->h[0],b=s->h[1],c=s->h[2],d=s->h[3],e=s->h[4],f=s->h[5],g=s->h[6],h=s->h[7];
 for(int i=0;i<64;i++){uint32_t S1=rr(e,6)^rr(e,11)^rr(e,25),ch=(e&f)^((~e)&g),t1=h+S1+ch+k[i]+w[i],S0=rr(a,2)^rr(a,13)^rr(a,22),maj=(a&b)^(a&c)^(b&c),t2=S0+maj;h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}
 s->h[0]+=a;s->h[1]+=b;s->h[2]+=c;s->h[3]+=d;s->h[4]+=e;s->h[5]+=f;s->h[6]+=g;s->h[7]+=h;
}
static void sha_init(SHA*s){uint32_t h[8]={0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};memcpy(s->h,h,sizeof h);s->bits=0;s->n=0;}
static void sha_add(SHA*s,const void*vp,size_t n){const uint8_t*p=vp;s->bits+=(uint64_t)n*8;while(n){size_t q=64-s->n;if(q>n)q=n;memcpy(s->b+s->n,p,q);s->n+=q;p+=q;n-=q;if(s->n==64){sha_block(s,s->b);s->n=0;}}}
static void sha_end(SHA*s,uint8_t out[32]){uint64_t bits=s->bits;s->b[s->n++]=0x80;if(s->n>56){while(s->n<64)s->b[s->n++]=0;sha_block(s,s->b);s->n=0;}while(s->n<56)s->b[s->n++]=0;for(int i=7;i>=0;i--)s->b[s->n++]=(uint8_t)(bits>>(i*8));sha_block(s,s->b);for(int i=0;i<8;i++){out[i*4]=(uint8_t)(s->h[i]>>24);out[i*4+1]=(uint8_t)(s->h[i]>>16);out[i*4+2]=(uint8_t)(s->h[i]>>8);out[i*4+3]=(uint8_t)s->h[i];}}
static void sha_hex(const void*p,size_t n,char out[65]){static const char*d="0123456789abcdef";SHA s;uint8_t h[32];sha_init(&s);sha_add(&s,p,n);sha_end(&s,h);for(int i=0;i<32;i++){out[i*2]=d[h[i]>>4];out[i*2+1]=d[h[i]&15];}out[64]=0;}

static Buf parse_bits_text(const uint8_t *p,size_t n,int annotated){ Buf o={0}; unsigned v=0,k=0; int in_comment=0; for(size_t i=0;i<n;i++){unsigned char c=p[i]; if(annotated){ if(c=='\n'){in_comment=0;continue;} if(in_comment)continue; if(c=='|'||c=='#'){in_comment=1;continue;} }
   if(c=='0'||c=='1'){v=(v<<1)|(unsigned)(c-'0');k++;if(k==8){put8(&o,(uint8_t)v);v=0;k=0;}} else if(isspace(c)){} else {fprintf(stderr,"bitweave: invalid character at byte %zu: 0x%02x\n",i,(unsigned)c);exit(2);} }
 if(k)die("bit count is not divisible by 8");
 return o; }
static void bits_to_file(const char*path,const uint8_t*p,size_t n){Buf o={0};for(size_t i=0;i<n;i++){for(int b=7;b>=0;b--)put8(&o,(uint8_t)(((p[i]>>b)&1)?'1':'0'));put8(&o,(uint8_t)(((i+1)%8)?' ':'\n'));}if(n%8)put8(&o,'\n');write_file(path,o.p,o.n);free(o.p);}
static const char* regs[16]={"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
static int regid(const char*s){for(int i=0;i<16;i++)if(!strcmp(s,regs[i]))return i;return -1;}
static char *trim(char*s){while(isspace((unsigned char)*s))s++;char*e=s+strlen(s);while(e>s&&isspace((unsigned char)e[-1]))*--e=0;return s;}
static long long num(const char*s){char*e=0;errno=0;long long v=strtoll(s,&e,0);if(errno||!e||*trim(e))die2("invalid integer",s);return v;}
static void rex(Buf*b,int w,int r,int x,int m){uint8_t q=0x40|(w?8:0)|(r?4:0)|(x?2:0)|(m?1:0);if(q!=0x40)put8(b,q);}
static void modrm_rr(Buf*b,int reg,int rm){put8(b,(uint8_t)(0xC0|((reg&7)<<3)|(rm&7)));}
static void append_literal_bits(Buf*b,const char*s){unsigned v=0,k=0;for(;*s;s++){unsigned char c=(unsigned char)*s;if(c=='0'||c=='1'){v=(v<<1)|(unsigned)(c-'0');if(++k==8){put8(b,(uint8_t)v);v=0;k=0;}}else if(isspace(c)){}else die2("invalid literal bits",s);}if(k)die("literal bits are not byte aligned");}

typedef struct{char name[64];size_t off;} Label;
typedef struct{Label v[512];size_t n;} Labels;
static int label_get(const Labels*l,const char*n,size_t*out){for(size_t i=0;i<l->n;i++)if(!strcmp(l->v[i].name,n)){*out=l->v[i].off;return 1;}return 0;}
static void label_put(Labels*l,const char*n,size_t off){size_t q=0;if(label_get(l,n,&q)){if(q!=off)die2("duplicate label",n);return;}if(l->n>=512)die("too many labels");if(!*n||strlen(n)>=sizeof(l->v[0].name))die2("invalid label",n);strcpy(l->v[l->n].name,n);l->v[l->n].off=off;l->n++;}
static void rel32(Buf*b,long long d){if(d<INT32_MIN||d>INT32_MAX)die("relative target out of range");put32(b,(uint32_t)(int32_t)d);}
static int cc_id(const char*s){static const char*n[]={"jo","jno","jb","jae","je","jne","jbe","ja","js","jns","jp","jnp","jl","jge","jle","jg"};if(!strcmp(s,"jz"))s="je";if(!strcmp(s,"jnz"))s="jne";for(int i=0;i<16;i++)if(!strcmp(s,n[i]))return i;return -1;}
static int cmov_cc(const char*s){static const char*n[]={"cmovo","cmovno","cmovb","cmovae","cmove","cmovne","cmovbe","cmova","cmovs","cmovns","cmovp","cmovnp","cmovl","cmovge","cmovle","cmovg"};for(int i=0;i<16;i++)if(!strcmp(s,n[i]))return i;return -1;}

typedef struct{int kind;int base;long long disp;char label[64];} Mem; /* kind 1=base+disp, 2=rip+label */
static int parse_mem(const char*s,Mem*m){memset(m,0,sizeof *m);size_t n=strlen(s);if(n<3||s[0]!='['||s[n-1]!=']')return 0;char t[128];if(n-2>=sizeof t)die("memory operand too long");memcpy(t,s+1,n-2);t[n-2]=0;char*q=trim(t);if(!strncmp(q,"rip+",4)||!strncmp(q,"rip-",4)){m->kind=2;int neg=q[3]=='-';q=trim(q+4);if(!*q||strlen(q)>=sizeof m->label)die2("invalid RIP label",q);strcpy(m->label,q);m->disp=neg?-1:1;return 1;}char*sign=0;for(char*x=q+1;*x;x++)if(*x=='+'||*x=='-'){sign=x;break;}if(sign){char sg=*sign;*sign=0;m->base=regid(trim(q));if(m->base<0)return 0;m->disp=num(trim(sign+1));if(sg=='-')m->disp=-m->disp;}else{m->base=regid(q);if(m->base<0)return 0;m->disp=0;}m->kind=1;return 1;}
static void emit_mem_modrm(Buf*b,int reg,const Mem*m,const Labels*l,size_t next_after_disp,int pass){if(m->kind==2){put8(b,(uint8_t)(((reg&7)<<3)|5));size_t dst=0;if(!label_get(l,m->label,&dst)){if(pass==2)die2("unresolved label",m->label);put32(b,0);return;}long long d=(long long)dst-(long long)next_after_disp;if(m->disp<0)d=-d;rel32(b,d);return;}int base=m->base;put8(b,(uint8_t)(0x80|((reg&7)<<3)|((base&7)==4?4:(base&7))));if((base&7)==4)put8(b,(uint8_t)(0x20|(base&7)));put32(b,(uint32_t)(int32_t)m->disp);}
static size_t mem_tail_bytes(const Mem*m){return m->kind==2?5:((m->base&7)==4?6:5);}

static void encode_line(Buf*b,char*line,Labels*l,int pass){char*c=strchr(line,'#');if(c)*c=0;line=trim(line);if(!*line)return;size_t ln=strlen(line);if(line[ln-1]==':'){line[ln-1]=0;char*n=trim(line);if(pass==1)label_put(l,n,b->n);return;}if(!strncmp(line,"bits ",5)){append_literal_bits(b,trim(line+5));return;}
 char op[32]={0},a[128]={0},d[128]={0};int z=sscanf(line,"%31s %127[^,], %127[^\n]",op,a,d);char one[128]={0};int z1=sscanf(line,"%31s %127[^\n]",op,one);if(!strcmp(op,"syscall")){put8(b,0x0f);put8(b,0x05);return;}if(!strcmp(op,"ret")){put8(b,0xc3);return;}if(!strcmp(op,"nop")){put8(b,0x90);return;}if(!strcmp(op,"leave")){put8(b,0xc9);return;}
 if((!strcmp(op,"jmp")||!strcmp(op,"call")||cc_id(op)>=0)&&z1==2){char*t=trim(one);size_t dst=0;int found=label_get(l,t,&dst);if(pass==2&&!found)die2("unresolved label",t);if(!strcmp(op,"jmp")||!strcmp(op,"call")){put8(b,!strcmp(op,"jmp")?0xe9:0xe8);long long r=found?(long long)dst-(long long)(b->n+4):0;rel32(b,r);return;}int cc=cc_id(op);put8(b,0x0f);put8(b,(uint8_t)(0x80+cc));long long r=found?(long long)dst-(long long)(b->n+4):0;rel32(b,r);return;}
 if((!strcmp(op,"inc")||!strcmp(op,"dec")||!strcmp(op,"push")||!strcmp(op,"pop")||!strcmp(op,"neg")||!strcmp(op,"not"))&&z1==2){int r=regid(trim(one));if(r<0)die2("unknown register",one);if(!strcmp(op,"push")||!strcmp(op,"pop")){if(r>=8)rex(b,0,0,0,1);put8(b,(uint8_t)((!strcmp(op,"push")?0x50:0x58)+(r&7)));return;}rex(b,1,0,0,r>>3);put8(b,!strcmp(op,"neg")||!strcmp(op,"not")?0xf7:0xff);modrm_rr(b,!strcmp(op,"inc")?0:!strcmp(op,"dec")?1:!strcmp(op,"not")?2:3,r);return;}
 if(z==3){char*aa=trim(a);char*dd=trim(d);int ra=regid(aa),rd=regid(dd);Mem ma,md;int hma=parse_mem(aa,&ma),hmd=parse_mem(dd,&md);
  if(!strcmp(op,"mov")){if(ra>=0&&rd>=0){rex(b,1,rd>>3,0,ra>>3);put8(b,0x89);modrm_rr(b,rd,ra);return;}if(ra>=0&&!hmd){rex(b,1,0,0,ra>>3);put8(b,(uint8_t)(0xb8+(ra&7)));put64(b,(uint64_t)num(dd));return;}if(ra>=0&&hmd){rex(b,1,ra>>3,0,hmd&&md.kind==1?(md.base>>3):0);put8(b,0x8b);size_t tail=mem_tail_bytes(&md);emit_mem_modrm(b,ra,&md,l,b->n+tail,pass);return;}if(hma&&rd>=0){rex(b,1,rd>>3,0,hma&&ma.kind==1?(ma.base>>3):0);put8(b,0x89);size_t tail=mem_tail_bytes(&ma);emit_mem_modrm(b,rd,&ma,l,b->n+tail,pass);return;}die2("invalid mov operands",line);}
  if(!strcmp(op,"lea")&&ra>=0&&hmd){rex(b,1,ra>>3,0,md.kind==1?(md.base>>3):0);put8(b,0x8d);size_t tail=mem_tail_bytes(&md);emit_mem_modrm(b,ra,&md,l,b->n+tail,pass);return;}
  if((!strcmp(op,"movzx")||!strcmp(op,"movsx"))&&ra>=0&&hmd){rex(b,1,ra>>3,0,md.kind==1?(md.base>>3):0);put8(b,0x0f);put8(b,!strcmp(op,"movzx")?0xb6:0xbe);size_t tail=mem_tail_bytes(&md);emit_mem_modrm(b,ra,&md,l,b->n+tail,pass);return;}
  int cc=cmov_cc(op);if(cc>=0&&ra>=0&&rd>=0){rex(b,1,ra>>3,0,rd>>3);put8(b,0x0f);put8(b,(uint8_t)(0x40+cc));modrm_rr(b,ra,rd);return;}
  if(!strcmp(op,"imul")&&ra>=0&&rd>=0){rex(b,1,ra>>3,0,rd>>3);put8(b,0x0f);put8(b,0xaf);modrm_rr(b,ra,rd);return;}
  if((!strcmp(op,"shl")||!strcmp(op,"shr")||!strcmp(op,"sar"))&&ra>=0){rex(b,1,0,0,ra>>3);put8(b,0xc1);modrm_rr(b,!strcmp(op,"shl")?4:!strcmp(op,"shr")?5:7,ra);put8(b,(uint8_t)num(dd));return;}
  if((!strcmp(op,"add")||!strcmp(op,"or")||!strcmp(op,"and")||!strcmp(op,"sub")||!strcmp(op,"xor")||!strcmp(op,"cmp")||!strcmp(op,"test"))&&ra>=0&&rd>=0){uint8_t opc=!strcmp(op,"add")?0x01:!strcmp(op,"or")?0x09:!strcmp(op,"and")?0x21:!strcmp(op,"sub")?0x29:!strcmp(op,"xor")?0x31:!strcmp(op,"cmp")?0x39:0x85;rex(b,1,rd>>3,0,ra>>3);put8(b,opc);modrm_rr(b,rd,ra);return;}
  if((!strcmp(op,"add")||!strcmp(op,"or")||!strcmp(op,"and")||!strcmp(op,"sub")||!strcmp(op,"xor")||!strcmp(op,"cmp"))&&ra>=0){int ext=!strcmp(op,"add")?0:!strcmp(op,"or")?1:!strcmp(op,"and")?4:!strcmp(op,"sub")?5:!strcmp(op,"xor")?6:7;rex(b,1,0,0,ra>>3);put8(b,0x81);modrm_rr(b,ext,ra);put32(b,(uint32_t)num(dd));return;}
 }
 fprintf(stderr,"bitweave: unsupported x86 lens line: %s\n",line);exit(2);
}
static Buf assemble_x86(const uint8_t*txt,size_t n){char*src=malloc(n+1);if(!src)die("oom");if(n)memcpy(src,txt,n);src[n]=0;Labels l={0};Buf first={0};char*copy=strdup(src);if(!copy)die("oom");char*save=0;for(char*line=strtok_r(copy,"\n",&save);line;line=strtok_r(0,"\n",&save))encode_line(&first,line,&l,1);free(first.p);free(copy);Buf out={0};copy=strdup(src);if(!copy)die("oom");save=0;for(char*line=strtok_r(copy,"\n",&save);line;line=strtok_r(0,"\n",&save))encode_line(&out,line,&l,2);free(copy);free(src);return out;}
static Buf elf64_wrap(const uint8_t*code,size_t n,int pie){const size_t off=4096;Buf b={0};grow(&b,off+n);memset(b.p,0,off+n);b.n=off+n;uint8_t *e=b.p;e[0]=0x7f;e[1]='E';e[2]='L';e[3]='F';e[4]=2;e[5]=1;e[6]=1; e[16]=pie?3:2;e[18]=62;e[20]=1;uint64_t base=pie?0:0x400000ULL;patch64(&b,24,base+off);patch64(&b,32,64);patch16(&b,52,64);patch16(&b,54,56);patch16(&b,56,1);size_t ph=64;patch32(&b,ph+0,1);patch32(&b,ph+4,5);patch64(&b,ph+8,0);patch64(&b,ph+16,base);patch64(&b,ph+24,base);patch64(&b,ph+32,b.n);patch64(&b,ph+40,b.n);patch64(&b,ph+48,4096);if(n)memcpy(b.p+off,code,n);return b;}
static size_t alignup(size_t x,size_t a){return (x+a-1)&~(a-1);}
static Buf pe64_wrap(const uint8_t*code,size_t n){const size_t hdr=512, raw=alignup(n,512);Buf b={0};grow(&b,hdr+raw);memset(b.p,0,hdr+raw);b.n=hdr+raw;uint8_t*e=b.p;e[0]='M';e[1]='Z';patch32(&b,0x3c,0x80);size_t p=0x80;e[p]='P';e[p+1]='E';p+=4;patch16(&b,p+0,0x8664);patch16(&b,p+2,1);patch16(&b,p+16,0x00f0);patch16(&b,p+18,0x0022);size_t o=p+20;patch16(&b,o+0,0x20b);e[o+2]=14;patch32(&b,o+4,(uint32_t)raw);patch32(&b,o+16,0x1000);patch32(&b,o+20,0x1000);patch64(&b,o+24,0x140000000ULL);patch32(&b,o+32,0x1000);patch32(&b,o+36,0x200);patch16(&b,o+40,6);patch16(&b,o+48,6);patch32(&b,o+56,0x2000);patch32(&b,o+60,0x200);patch16(&b,o+68,3);patch16(&b,o+70,0x8100);patch64(&b,o+72,1ULL<<20);patch64(&b,o+80,4096);patch64(&b,o+88,1ULL<<20);patch64(&b,o+96,4096);patch32(&b,o+108,16);size_t sh=o+0xf0;memcpy(e+sh,".text",5);patch32(&b,sh+8,(uint32_t)n);patch32(&b,sh+12,0x1000);patch32(&b,sh+16,(uint32_t)raw);patch32(&b,sh+20,0x200);patch32(&b,sh+36,0x60000020);if(n)memcpy(e+0x200,code,n);return b;}

static void cmd_manifest(const char*file,const char*target,const char*kind){Buf b=read_file(file);char h[65];sha_hex(b.p,b.n,h);printf("BITWEAVE-MANIFEST-2\nbytes %zu\nsha256 %s\ntarget %s\nkind %s\n",b.n,h,target?target:"unspecified",kind?kind:"native");free(b.p);}

static void usage(void){puts("BitWeave " BW_VERSION " — forward native binary authoring\n"
"commands:\n"
"  build-bits <source.bwx> <out.bits>\n"
"  build-linux-exec <source.bwx> <out.elf> [out-full.bits]\n"
"  build-linux-pie <source.bwx> <out.pie> [out-full.bits]\n"
"  build-win64 <source.bwx> <out.exe> [out-full.bits]\n"
"  pack <in.bits> <out.bin>\n"
"  canon <in.bwa> <out.bits>\n"
"  verify <in.bits> <built-file>\n"
"  sha256 <file>\n"
"  manifest <file> [target] [kind]\n"
"  abi <sysv-amd64|linux-x86_64-syscall|win64>\n"
"  ai-author-prompt <brief.txt>\n");}

static void abi(const char*x){if(!strcmp(x,"sysv-amd64"))puts("args rdi rsi rdx rcx r8 r9\nreturn rax\ncallee-saved rbx rbp r12 r13 r14 r15\nstack alignment 16 before call");else if(!strcmp(x,"linux-x86_64-syscall"))puts("number rax\nargs rdi rsi rdx r10 r8 r9\nreturn rax\nclobber rcx r11");else if(!strcmp(x,"win64"))puts("integer-args rcx rdx r8 r9\nreturn rax\nshadow-space 32\nstack alignment 16\ncallee-saved rbx rbp rdi rsi rsp r12 r13 r14 r15");else die2("unknown ABI",x);}

static void ai_author_prompt(const char*path){Buf b=read_file(path);fputs("BITWEAVE-AI-AUTHOR-1\nCreate a new BitWeave .bwx program from the brief below.\nReturn only complete .bwx source.\nUse labels, supported native instructions, and literal `bits ...` data when needed.\nDo not return hex. Do not refer to or require any pre-existing executable.\nThe result will be built forward by BitWeave.\nBRIEF-BEGIN\n",stdout);fwrite(b.p,1,b.n,stdout);fputs("\nBRIEF-END\n",stdout);free(b.p);}

static Buf build_source_bits(const char*path){Buf t=read_file(path);Buf b=assemble_x86(t.p,t.n);free(t.p);return b;}
static void write_container(const char*kind,const char*source,const char*out,const char*bitsout){Buf c=build_source_bits(source);Buf o=!strcmp(kind,"win64")?pe64_wrap(c.p,c.n):elf64_wrap(c.p,c.n,!strcmp(kind,"linux-pie"));write_file(out,o.p,o.n);if(bitsout)bits_to_file(bitsout,o.p,o.n);if(strcmp(kind,"win64"))chmod(out,0755);free(c.p);free(o.p);}

int main(int ac,char**av){
 if(ac<2){usage();return 0;}
 if(!strcmp(av[1],"--version")||!strcmp(av[1],"version")){puts(BW_VERSION);return 0;}
 if(!strcmp(av[1],"build-bits")&&ac==4){Buf b=build_source_bits(av[2]);bits_to_file(av[3],b.p,b.n);free(b.p);return 0;}
 if(!strcmp(av[1],"build-linux-exec")&&(ac==4||ac==5)){write_container("linux-exec",av[2],av[3],ac==5?av[4]:0);return 0;}
 if(!strcmp(av[1],"build-linux-pie")&&(ac==4||ac==5)){write_container("linux-pie",av[2],av[3],ac==5?av[4]:0);return 0;}
 if(!strcmp(av[1],"build-win64")&&(ac==4||ac==5)){write_container("win64",av[2],av[3],ac==5?av[4]:0);return 0;}
 if(!strcmp(av[1],"pack")&&ac==4){Buf t=read_file(av[2]),b=parse_bits_text(t.p,t.n,0);write_file(av[3],b.p,b.n);free(t.p);free(b.p);return 0;}
 if(!strcmp(av[1],"canon")&&ac==4){Buf t=read_file(av[2]),b=parse_bits_text(t.p,t.n,1);bits_to_file(av[3],b.p,b.n);free(t.p);free(b.p);return 0;}
 if(!strcmp(av[1],"verify")&&ac==4){Buf t=read_file(av[2]),a=parse_bits_text(t.p,t.n,0),b=read_file(av[3]);int ok=a.n==b.n&&(a.n==0||!memcmp(a.p,b.p,a.n));printf("%s bytes=%zu\n",ok?"PASS":"FAIL",a.n);free(t.p);free(a.p);free(b.p);return ok?0:1;}
 if(!strcmp(av[1],"sha256")&&ac==3){Buf b=read_file(av[2]);char h[65];sha_hex(b.p,b.n,h);puts(h);free(b.p);return 0;}
 if(!strcmp(av[1],"manifest")&&ac>=3&&ac<=5){cmd_manifest(av[2],ac>3?av[3]:0,ac>4?av[4]:0);return 0;}
 if(!strcmp(av[1],"abi")&&ac==3){abi(av[2]);return 0;}
 if(!strcmp(av[1],"ai-author-prompt")&&ac==3){ai_author_prompt(av[2]);return 0;}
 usage();return 2;
}
