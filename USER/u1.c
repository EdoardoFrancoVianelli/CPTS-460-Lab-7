#include "ucode.c"

#include <time.h>

int color;

int itimer();

int part3 = 1;


main(int argc, char *argv[])
{ 
	int i, j, pid, ppid, _time;
	char name[64]; int cmd, segment;

	syscall(12, part3, 0, 0);
	syscall(14, time(NULL), 0, 0);

	if (part3 == 0){

		pid = getpid();
		ppid = getppid();
		
		while (1){
			printf("pid = [%d] ppid = [%d]\n", pid, ppid);
						
			//large delay loop
			while (1){  }
		}
		
		printf("exit u1\n");
	}else{
		while(1){
			pid = getpid();
			color = 0x000B + (pid % 5);
			segment = (pid+1)*0x1000;   
			printf("==============================================\n");
			printf("I am proc %din U mode: segment=%x\n", pid, segment);
			show_menu();
			printf("Command ? ");
			my_gets(name); 
			if (name[0]==0) 
				continue;

			cmd = find_cmd(name);

			switch(cmd){
				case 0 : getpid();   break;
				case 1 : ps();       break;
				case 2 : chname();   break;
				case 3 : kmode();    break;
				case 4 : kswitch();  break;
				case 5 : wait();     break;

				case 6 : exit();     break;
				case 7 : fork();     break;
				case 8 : exec();     break;
				//case 9 : do_vfork(); break;
				case 9 : itimer();  break;

				default: invalid(name); break;
			} 
		}
	}
}


