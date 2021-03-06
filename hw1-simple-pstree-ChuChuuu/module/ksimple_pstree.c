#include <linux/module.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/signal.h>
#include <linux/sched/signal.h>
#define NETLINK_TEST 30

struct sock *kn_sk = NULL;

void nl_custom_data_ready(struct sk_buff *skb);
void FindChild(int default_pid, int level,int userpid,char* data);
void FindSib(int defauld_pid,int UserPid,char* data);
int FindPar(int defauld_pid,int level,int UserPid,char* data);
char TransData[9500];
void sending(char* data,int dstPID);

int  __init nl_custom_init(void)
{
    struct netlink_kernel_cfg nlcfg = {
        .input = nl_custom_data_ready,
    };

    kn_sk = netlink_kernel_create(&init_net,NETLINK_TEST,&nlcfg);
    printk("kernel:initial ok!\n");
    if(!kn_sk) {
        printk("kernel:netlink create error\n");
    }

    return 0;
}
void __exit nl_custom_exit(void)
{
    printk("kernel:close\n");
    netlink_kernel_release(kn_sk);
    return ;
}


void sending(char* data, int dstPID)
{
    struct nlmsghdr *out_nlh;
    struct sk_buff *out_skb;
    void *out_payload;
    int outpayload_len = 10000;
    out_skb = nlmsg_new(outpayload_len,GFP_KERNEL);
    if(!out_skb) {
        printk("fail out skb\n");
    }
    out_nlh = nlmsg_put(out_skb,0,0,0,outpayload_len,0);
    if(!out_nlh) {
        printk("fail in data ready\n");
    }
    out_payload = nlmsg_data(out_nlh);
    strcpy(out_payload,data);
    netlink_unicast(kn_sk,out_skb,dstPID,MSG_DONTWAIT);

    return;
}
void nl_custom_data_ready(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    void *payload;
    char mode;
    int givenpid;
    struct pid *pidtest;
    strcpy(TransData,"");
    nlh = nlmsg_hdr(skb);


    payload = nlmsg_data(nlh);
    sscanf((char *)payload,"%c%d",&mode,&givenpid);
    printk("kernel:i get : %c %d \n",mode,givenpid);

    pidtest = find_get_pid(givenpid);
    if(!pidtest) {
        mode = 'n';
    }

    switch(mode) {
    case 'c':
        printk("c\n");
        FindChild(givenpid,0,nlh->nlmsg_pid,TransData);
        break;
    case 's':
        printk("s\n");
        FindSib(givenpid,nlh->nlmsg_pid,TransData);
        break;
    case 'p':
        printk("p\n");
        FindPar(givenpid,0,nlh->nlmsg_pid,TransData);
    case 'n':
        break;
    default:
        break;
    }

    sending(TransData,nlh->nlmsg_pid);
    strcpy(TransData,"");
    return;
}
void FindChild(int default_pid,int level,int userpid,char* data)
{
    pid_t pid = (pid_t)default_pid;

    struct task_struct *p;
    struct list_head *pp = NULL;
    struct task_struct *psibling;
    //char space[150]="";
    char namepid[70]="";
    int i = 0;



    p = pid_task(find_get_pid(pid),PIDTYPE_PID);

    for(i = 0 ; i < level ; i++) {
        strcat(data,"    ");
    }

    sprintf(namepid,"%s(%d)\n",p->comm,p->pid);
    strcat(data,namepid);
    //sending(space,userpid);
    //strcpy(space,"");
    strcpy(namepid,"");
    //printk("level: %d this: %s %d...%d..%d\n",level,p->comm,p->pid,sizeof(p->comm),sizeof(p->pid));
    level++;
    //printk("%p\n",&p->children);
    list_for_each(pp,&p->children) {
        psibling = list_entry(pp,struct task_struct,sibling);
        FindChild((int)psibling->pid,level,userpid,data);
    }
    return;
}

void FindSib(int default_pid,int UserPid,char* data)
{
    pid_t pid = (pid_t)default_pid;
    struct task_struct *p;
    struct list_head *pp = NULL;
    struct task_struct *psibling;
    char space[150]="";

    p = pid_task(find_get_pid(pid),PIDTYPE_PID);

    list_for_each(pp,&p->parent->children) {
        psibling = list_entry(pp,struct task_struct,sibling);
        if(psibling->pid == default_pid) {
            continue;
        }
        sprintf(space,"%s(%d)\n",psibling->comm,psibling->pid);
        strcat(data,space);
        //sending(space,UserPid);
        strcpy(space,"");
    }
    return;
}

int FindPar(int default_pid,int level,int UserPid,char* data)
{
    pid_t pid = (pid_t)default_pid;

    struct task_struct *p;
    int i;
    //char space[150]="";
    char namepid[70]="";

    p = pid_task(find_get_pid(pid),PIDTYPE_PID);

    if(p->parent->pid != 0) {
        level = FindPar(p->parent->pid,level,UserPid,data);
    }

    for(i = 0 ; i < level ; i++) {
        strcat(data,"    ");
    }
    sprintf(namepid,"%s(%d)\n",p->comm,p->pid);
    //printk("%s %d\n",p->comm,p->pid);
    strcat(data,namepid);
    //sending(space,UserPid);
    //strcpy(space,"");
    strcpy(namepid,"");

    level++;
    return level;
}


module_init(nl_custom_init);
module_exit(nl_custom_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("test");
MODULE_AUTHOR("chu");
