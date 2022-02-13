#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>	   //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define TRUE 1
#define FALSE 0

//------------------Global variable-------------------
struct Packet
{
	int found; //1---TRUE; 0----FALSE; additional function -3 meaning that I just want to ask u about your name 3 for name respone
	char destination[60];
	int path[500];
	int cur_i; //current index of path[]
	int client_socket;
	struct tm time_request;
	char response[300];
};
struct Station
{
	char name[50];
	char destination[60];
	int udp_socket;
	int tcp_socket; //should use port 2222
	int udp;
	int udp_adj[500]; //add adjadency udp port here//need to use malloc to resize...
	int num_udp_adj;
	char udp_adj_name[500][30]; // store name of adj UDP
	char response[300];			//store respone here
};
/***********************************/
int i; //use in for loop
struct Station station;
struct Packet packet;
int char_count;
static const struct Packet EmptyStruct;
struct sockaddr_in address, address2, address3;
int new_socket;
int addrlen;
char *tt_file;		   //store the time table in here
char file[80] = "tt-"; //store the whole name of file here
struct stat stat1;	   //determine modification of file;

//set of socket descriptors
fd_set readfds;
//----------------------------------------------------
int max(int x, int y)
{
	if (x > y)
		return x;
	else
		return y;
}
int accept_sd()
{ //on sucess return 1
	if ((new_socket = accept(station.tcp_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
	{
		perror("In Accept: ");
		exit(EXIT_FAILURE);
	}
	//inform user of socket number - used in send and receive commands
	printf("New connection, socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	return 1;
}
//create address for tcp socket or udp socket
//addr_type == 1 > for tcp socket
//addr_type == 2 > for udp socket
//addr_type == 3 > for sendto udp socket
int create_addr(int port, int addr_type)
{
	if (addr_type == 1)
	{
		//type of socket created
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);
		printf("TCP on port: %d\n", port);
		return 1;
	}
	else if (addr_type == 2)
	{
		//type of UDP socket created
		address2.sin_family = AF_INET;
		address2.sin_addr.s_addr = INADDR_ANY;
		address2.sin_port = htons(port);
		printf("UDP on port: %d\n", port);
		return 1;
	}
	else if (addr_type == 3)
	{
		//type of UDP socket created
		memset(address3.sin_zero, '\0', sizeof address3.sin_zero);
		address3.sin_family = AF_INET;
		address3.sin_addr.s_addr = INADDR_ANY;
		address3.sin_port = htons(port);
		return 1;
	}
	return -1;
}
void tt_handle(char *name)
{

	strcat(file, name);
	strcat(file, ".txt"); //remember to remove this line***************************************************************************
	printf("file name: %s\n", file);
	stat(file, &stat1);
	printf("Lastest modification of file: %s\n", ctime(&stat1.st_mtime));
	FILE *time_table;
	time_table = fopen(file, "r");
	if (time_table == NULL)
	{
		perror("Error opening file\n");
		exit(EXIT_FAILURE);
	}
	char *pStr;
	size_t len_max = 2;
	unsigned int current_size = 0;
	int ch_count;
	pStr = (char *)malloc(len_max * sizeof(char));
	if (pStr == NULL)
	{
		printf("Could not allocated memory to pStr\n");
		free(pStr);
		exit(0);
	}
	tt_file = (char *)malloc(len_max * sizeof(char));
	if (tt_file == NULL)
	{
		printf("Could not allocated memory to tt_file\n");
		free(tt_file);
		exit(0);
	}
	int z = 0;
	int amount = 0;
	//int line_count=0;
	while ((ch_count = getline(&pStr, &len_max, time_table)) != -1)
	{
		//line_count++;
		amount = amount + ch_count;
		tt_file = (char *)realloc(tt_file, amount);
		if (tt_file == NULL)
		{
			printf("Could not RE-allocated memory to....\n");
			free(tt_file);
			exit(0);
		}
		if (pStr[ch_count - 1] != '\0' || pStr[ch_count - 1] != '\n')
		{
			
			current_size = ch_count * 2;
			//printf("the current size: %d\n",current_size);
			pStr = (char *)realloc(pStr, current_size);

			if (pStr == NULL)
			{
				printf("Could not RE-allocated memory to pStr\n");
				free(pStr);
				exit(0);
			}
		}
		printf("> %s", pStr);
		for (int index = 0; index < ch_count; index++)
		{
			tt_file[z] = pStr[index];
			z++;
		}
	}
	char_count = z + 1;
	//printf("line count: %d\n",line_count);
	printf("Free the pointers\n");
	free(pStr);
	//free(tt_file);
	printf("Closing the file\n");
	fclose(time_table);
} //close of tt_handle

int destination_handle(struct tm timeinfo, char *destination)
{ //we have destination and you have to do sth with it

	time_t t2 = mktime(&timeinfo);
	struct stat stat2;
	stat(file, &stat2);
	printf("Chekcing modification day of file: %s\n", ctime(&stat2.st_mtime));
	if (stat1.st_mtime == stat2.st_mtime)
	{
		printf("No new modification so far, Relief!!!\n");
	}
	else
	{
		printf("free tt_file and handle timetable file again\n");
		free(tt_file);
		tt_handle(station.name);
	}
	//meaning there will be always respone whether there is timetable or not
	printf("checking destination:-%s", destination);
	char *copy_tt_file = (char *)malloc(char_count * sizeof(char));
	if (copy_tt_file == NULL)
	{
		printf("Could not allocated memory to tt_file\n");
		free(copy_tt_file);
		exit(0);
	}
	strcpy(copy_tt_file, tt_file);

	if (strstr(tt_file, destination) != NULL)
	{
		printf("There is destination in our neighbourhood :D\n");

		char *saveptr1, *saveptr2;
		char *line = strtok_r(copy_tt_file, "\n", &saveptr1);
		line = strtok_r(NULL, "\n", &saveptr1); //statring with the second line
		//printf("the second line: %s",line);
		while (line != NULL)
		{
			//printf("\n>%s\n",line);
			char copy_line[200];
			for (i = 0; i < strlen(line); i++)
			{
				copy_line[i] = line[i];
			}
			//copy_line[i]='\0';
			char *tt_depart = strtok_r(line, ",", &saveptr2);
			struct tm tm_tt_depart; //time in timetable
			memset(&tm_tt_depart, 0, sizeof(struct tm));
			strptime(tt_depart, "%H:%M", &tm_tt_depart);
			tm_tt_depart.tm_year = timeinfo.tm_year;
			tm_tt_depart.tm_mon = timeinfo.tm_mon;
			tm_tt_depart.tm_mday = timeinfo.tm_mday;
			time_t time1 = mktime(&tm_tt_depart); //to compare ?
			if (time1 > t2)						  //t2 is the time of request
			{
				printf("tm_tt_depart is bigger\n");
				//printf("\ntm_tt_depart: year- %d [%d:%d]\n\n",tm_tt_depart.tm_year,tm_tt_depart.tm_hour, tm_tt_depart.tm_min);
				//More to do
				if (strstr(copy_line, destination) != NULL)
				{
					//printf("\nthe time table: %s\n",copy_line);
					//???TO DO
					memset(station.response, 0, 300);
					strcpy(station.response, copy_line);
					free(copy_tt_file);
					return 1; //I got time schedule for this destination in timetable
				}
			}
			//printf("Again: %s\n",line);
			//printf("****Copy: %s\n",copy_line);
			line = strtok_r(NULL, "\n", &saveptr1);
			//printf("***********: %s\n",line);
		}
		//printf("..Destination in our timetable but..TOO LATE..\n");
		memset(station.response, 0, 300);
		strcpy(station.response, "..Destination in our timetable but..TOO LATE..\n");
		free(copy_tt_file);
		return 1; //there is destination in timetable but it is too later now, plz return tmr
	}
	else
	{
		printf("I don't know such a destination in time table --> ask other UDP for more\n");
		free(copy_tt_file);
		return 0;
		//since there is not destination in our neighbourhood > ask udp about destination
	}
}

/*#######################################################################################################################################*/

int main(int argc, char *argv[])
{
	packet.cur_i = 0;
	int opt = TRUE;
	int client_socket[30], client_wait[30], max_clients = 30, activity, valread, sd;
	int max_sd;
	char buffer[1025]; //data buffer of 1K
	char station_name[50];
	strcpy(station.name, argv[1]);
	/*********************** Open File Handling ***********************************/
	tt_handle(station.name);
	/******************************************************************************/
	//a message
	char *message = "************ECHO Daemon v1.0 ***************** \r\n";

	printf("the name of station is: %s\n", station.name);
	/***************************** Adding neighborhood UDP ************************************/
	int x;
	for (x = 4; x < argc; x++)
	{
		station.udp_adj[x - 4] = atoi(argv[x]);
	}
	station.num_udp_adj = x - 4;
	printf("number of ajd upd is: %d\n", station.num_udp_adj);

	/**********************************************************************************************************************/
	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
		client_wait[i] = 0;
	}
	//******************************************* TCP ************************************************************************
	//create a master socket
	if ((station.tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	if (setsockopt(station.tcp_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
				   sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	
	int create_address = create_addr(atoi(argv[2]), 1);
	//bind the socket to localhost port
	if (bind(station.tcp_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	addrlen = sizeof(address); //assign length to addrlen for TCP address
	//try to specify maximum of 3 pending connections for the master socket
	if (listen(station.tcp_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	//***************************************************************************************************************************
	//************************************************ UDP **********************************************************************
	//create a UDP socket
	if ((station.udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	int create_address2 = create_addr(atoi(argv[3]), 2); //for UDP of this STATION server
	station.udp = atoi(argv[3]);
	if (bind(station.udp_socket, (struct sockaddr *)&address2, sizeof(address2)) < 0)
	{
		perror("UDP bind failed");
		exit(EXIT_FAILURE);
	}
	sleep(3); //sleep a bit to so that other server runs
	//----------------------------------------- ASKing for name ------------------------------------------
	for (i = 0; i < station.num_udp_adj; i++)
	{
		//printf("Asking for name to UDP:> %d\n",station.udp_adj[i]);
		packet.found = -3; //define the packet that this packet is for asking name

		printf("the adjancey udp number of this station: %d\n", station.udp_adj[i]);
		int create = create_addr(station.udp_adj[i], 3); //type 3 create UDP address to send to
		if (create == -1)
		{
			printf("CAnnot fucking create ADDress!!!\n");
		}
		int sen = sendto(station.udp_socket, &packet, sizeof(packet), 0, (struct sockaddr *)&address3, sizeof(address3));
		if (sen < 0)
		{
			printf("Holy Crap, Cant send to ask FOR UDP NAME\n");
		}
		else
		{
			//printf("Yeah we sent for ask\n");
		}
	}
	/****************************************** ASKing for name *************************************************/
	//************************************************************************************************************
	//accept the incoming connection

	puts("Waiting for connections ...");
	//********************************************************** MAIN LOOP ****************************************
	while (TRUE)
	{
		//clear the socket set
		FD_ZERO(&readfds);
		//add tcp socket to set
		FD_SET(station.tcp_socket, &readfds);
		//add udo socket to set
		FD_SET(station.udp_socket, &readfds);
		max_sd = max(station.tcp_socket, station.udp_socket);

		//add child sockets to set
		for (i = 0; i < max_clients; i++)
		{
			//socket descriptor
			sd = client_socket[i];

			//if valid socket descriptor then add to read list
			if (sd > 0)
				FD_SET(sd, &readfds);

			//highest file descriptor number, need it for the select function
			if (sd > max_sd)
				max_sd = sd;
		}

		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR))
		{
			printf("select error");
		}

		//If something happened on the master socket ,
		//then its an incoming connection
		if (FD_ISSET(station.tcp_socket, &readfds))
		{ //accept new connection
			int acct = accept_sd();

			//send new connection greeting message
			/* if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            {   
                perror("send");   
            }  
            puts("Welcome message sent successfully to TCP client");   
             */
			//add new socket to array of sockets
			for (i = 0; i < max_clients; i++)
			{
				//if position is empty
				if (client_socket[i] == 0)
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n", i);
					break;
				}
			}
		}
		//a message for UDP station server
		char *mess = "Greeting to other udp client \r\n";
		char *mess2 = "*****************\r\n";
		if (FD_ISSET(station.udp_socket, &readfds))
		{
			struct sockaddr_in addr;
			socklen_t len;
			len = sizeof(addr);
			struct Packet packet_recv;
			int n = recvfrom(station.udp_socket, &packet_recv, sizeof(packet_recv), MSG_WAITALL, (struct sockaddr *)&addr, &len);
			printf("Receiving from UDP port : %d \n", ntohs(addr.sin_port));
			if (n > 0 && packet_recv.found == -3)
			{
				printf("I got askking for name from UDP %d\n", ntohs(addr.sin_port));
				strcpy(packet_recv.response, station.name);
				packet_recv.found = 3;
				int s = sendto(station.udp_socket, &packet_recv, sizeof(packet_recv), 0, (struct sockaddr *)&addr, sizeof(addr));
				if (s < 0)
				{
					perror("ASKING name error: ");
				} //else{puts("We sent out NAME to UDP successfully");}
			}
			else if (n > 0 && packet_recv.found == 3)
			{
				//printf("Our NEIGHBOUR station name of %d is: %s\n",ntohs (addr.sin_port),packet_recv.response);
				for (i = 0; i < station.num_udp_adj; i++)
				{
					if (station.udp_adj[i] == ntohs(addr.sin_port))
					{
						strcpy(station.udp_adj_name[i], packet_recv.response);
						//printf("\nDOUBLE CHECK for Name: %s\n\n",station.udp_adj_name[i]);
					}
				} //end for
			}	  //end if for found==3
			else if (n > 0 && packet_recv.found == 0)
			{
				//TO DO here
				printf("I got REQUEST here\n;");
				int res = destination_handle(packet_recv.time_request, &packet_recv.destination[0]);
				if (res == 1)
				{
					packet_recv.found = 1; //we found the destination in our neighbour
					packet_recv.path[packet_recv.cur_i] = 0;
					packet_recv.cur_i--;
					//do not sure that the destination is available to tralvel but we happily to accept it
					if (strstr(station.response, packet_recv.destination) != NULL)
					{
						strcat(packet_recv.response, "----THEN----");
						strcat(packet_recv.response, station.response);
						int s = sendto(station.udp_socket, &packet_recv, sizeof(packet_recv), 0, (struct sockaddr *)&addr, sizeof(addr));
						if (s < 0)
						{
							perror("Return response error: ");
						}
						else
						{
							puts("We sent response UDP successfully");
						}
					}
					else
					{
						//got the destination but its t0o late;
						strcpy(packet_recv.response, "We found your destination but it is too late now, please get back tomorrow!\n");

						int s = sendto(station.udp_socket, &packet_recv, sizeof(packet_recv), 0, (struct sockaddr *)&addr, sizeof(addr));
						if (s < 0)
						{
							perror("Return response error: ");
						}
						else
						{
							puts("We sent response UDP successfully BUT it is TOO LATE to travel");
						}
					}
				}
				else if (res == 0) //I dun know any shit about destination-->ask my neighbours plz
				{
					//ASK other UDP
					int visited;
					for (int ix = 0; ix < station.num_udp_adj; ix++)
					{
						visited = 0; //false
						char name[30];
						printf("UDP neighbour %d we going to check before sending\n", station.udp_adj[ix]);
						for (int x = 0; x < packet_recv.cur_i + 1; x++)
						{
							printf("checking if neighbour UDP is inpath at %d is: %d\n", x, packet_recv.path[x]);
							if (packet_recv.path[x] == station.udp_adj[ix])
							{
								visited = 1; //true
								break;
							}
						}
						if (visited == 0)
						{
							printf("\nASKing to UDP number: %d at i %d\n\n", station.udp_adj[ix], ix);
							strcpy(name, station.udp_adj_name[ix]);
							//printf("can our neighbour name be copeid?: %s\n",name);
							int res = destination_handle(packet_recv.time_request, name); //determine other neighbour time to get there
							///return 1 but not guartee that there will be a time
							if (res == 1)
							{
								if (strstr(station.response, name) != NULL)
								{
									printf("--We can send REQUEST packet to %s UDP: %d by:\n----\n%s\n", name, station.udp_adj[ix], station.response);
									char *saveptr;
									char *time = strtok_r(station.response, ",", &saveptr);
									time = strtok_r(NULL, ",", &saveptr);
									time = strtok_r(NULL, ",", &saveptr);
									time = strtok_r(NULL, ",", &saveptr);

									struct tm tm_time; //time in timetable
									memset(&tm_time, 0, sizeof(struct tm));
									strptime(time, "%H:%M", &tm_time);
									tm_time.tm_year = packet_recv.time_request.tm_year;
									tm_time.tm_mon = packet_recv.time_request.tm_mon;
									tm_time.tm_mday = packet_recv.time_request.tm_mday;
									printf("time request->%d %d:%d\n", tm_time.tm_year, tm_time.tm_hour, tm_time.tm_min);
									packet_recv.time_request = tm_time;
									packet_recv.cur_i++;
									packet_recv.path[packet_recv.cur_i] = station.udp;
									int create = create_addr(station.udp_adj[ix], 3); //type 3 create UDP address to send to
									if (create == -1)
									{
										printf("CAnnot fucking create ADDress!!!\n");
									}

									int sen = sendto(station.udp_socket, &packet_recv, sizeof(packet_recv), 0, (struct sockaddr *)&address3, sizeof(address3));
									if (sen < 0)
									{
										printf("Holy Crap, Cant send REQUEST\n");
									}
									else
									{
										printf("Yeah we sent for REQUEST\n");
									}
								}
								else
								{
									//WE cant send to our neighbour UDP cause we late so ignore this UDP
								}
							} //end if res==1 in asking UDP
						}	  //end if visited==0

					} //end for
				}	  //end of res==0, end of asking other UDP
			}		  //end of found==0
			else if (n > 0 && packet_recv.found == 1)
			{
				printf("the path at 0: %d\n", packet_recv.path[0]);
				printf("our UDP station: %d\n", station.udp);
				if (station.udp == packet_recv.path[0])
				{

					char http_header[2048] = "HTTP/1.1 200 OK\nContent-type: text/html\r\n\n";
					strcat(http_header, packet_recv.response);
					int client = packet_recv.client_socket;
					send(client, http_header, strlen(http_header), 0);
					close(client);
					for (int i = 0; i < max_clients; i++)
					{
						if (client == client_socket[i])
						{
							client_socket[i] = 0;
						}
					}
				}
				else
				{
					//TO DO: sending packet backward the path
					printf("send backward our UDP station: %d\n", station.udp);
					printf("current index of path: %d and UDP port: %d\n", packet_recv.cur_i, packet_recv.path[packet_recv.cur_i]);
					int to = packet_recv.path[packet_recv.cur_i];
					if (packet_recv.cur_i != 0)
					{

						packet_recv.path[packet_recv.cur_i] = 0;
						packet_recv.cur_i--;
					}
					int create = create_addr(to, 3); //type 3 create UDP address to send to
					if (create == -1)
					{
						printf("<<<CAnnot fucking create ADDress to send BACKWARD!!!\n");
					}

					int sen = sendto(station.udp_socket, &packet_recv, sizeof(packet_recv), 0, (struct sockaddr *)&address3, sizeof(address3));
					if (sen < 0)
					{
						printf("Holy Crap, Cant send REQUEST\n");
					}
					else
					{
						printf("Yeah we sent for REQUEST\n");
					}
				}

			} //end of found==1
			else if (n < 0)
			{

				printf("error in recvfrom\n");
			}
		} //end if of receiving UDP
		//else its some IO operation on some other socket
		for (int i1 = 0; i1 < max_clients; i1++)
		{
			sd = client_socket[i1];

			if (FD_ISSET(sd, &readfds))
			{
				//incoming message
				if ((valread = read(sd, buffer, 1024)) <= 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
					//Close the socket and mark as 0 in list for reuse
					close(sd);
					client_socket[i1] = 0;
				} //Echo back the message that came in
				else
				{
					time_t rawtime;
					struct tm timeinfo; //time for request

					time(&rawtime);
					timeinfo = *localtime(&rawtime);

					//set the string terminating NULL byte on the end of the data read
					buffer[valread] = '\0';
					int aa = strlen(buffer);
					/***************************************************Special case to free tt_file***************************************************************/
					if (strstr(buffer, "free") != NULL)
					{
						printf("Freeing tt_file now\n");
						free(tt_file);
					}
					//printf("TCP client message: %s\n", buffer);
					char *token1, *token2, *des;

					token1 = strtok(buffer, "\n");
					printf("***First token: %s\n", token1);
					if (strstr(buffer, "favico") != NULL)
					{
						printf("FUck you This reQuesT, Get the FuCK oUt oFf heRE\n");
					}

					else if (strstr(token1, "GET") != NULL)
					{

						token2 = strtok(token1, " ");

						printf("token2 %s\n", token2);

						token2 = strtok(NULL, " ");

						printf("the second of token2 is: %s |len: %ld\n", token2, strlen(token2));
						token2[strlen(token2)] = '\0';

						char *p;
						if ((p = strstr(token2, "=")) != NULL)
						{
							//printf("may be there is des in herer\n");
							int pos = 0;
							for (i = 0; i <= strlen(token2); i++)
							{
								//printf("token2: %c\n\n",token2[i]);
								if (token2[i] == '=')
								{
									printf("found = at %d\n", i);
									des = &token2[i + 1];
									memset(station.destination, 0, strlen(station.destination));
									strcpy(station.destination, des);
									printf("the destination: %s\n", station.destination);
									break;
								} //end if
							}	  //end for

							printf("passing des: %s\n", station.destination);
							int resp = -1;												  //dont know anything, can't determine if destination is in timetable or not
							resp = destination_handle(timeinfo, &station.destination[0]); //if 1 ---> have reponse
							//if 0 ----> no respone, cannot found destination in this server'timetable
							if (resp == 1)
							{

								//we have respone but not know what it is
								//respone to tcp client now
								char http_header[2048] = "HTTP/1.1 200 OK\nContent-type: text/html\r\n\n";
								strcat(http_header, station.response);
								send(sd, http_header, strlen(http_header), 0);
								close(sd);
								client_socket[i] = 0;
								//close>>>reset client_wait to zero
							} //end if of response to TCP client at TCP
							else if (resp == 0)
							{
								//***************************************            .....ASK UDP.....   ****************************             *********************             ********************************
								printf("***prepare for asking UDP\n");
								packet.found = 0;
								packet.path[packet.cur_i] = station.udp;
								printf("our UDP %d\n", station.udp);
								printf("first UDP in path is: %d\n at cur_i: %d", packet.path[packet.cur_i], packet.cur_i);
								packet.client_socket = sd;
								memset(packet.destination, 0, strlen(packet.destination));
								strcpy(packet.destination, station.destination);
								for (int ix = 0; ix < station.num_udp_adj; ix++)
								{
									char name[30];
									printf("\nSending to UDP number: %d at i %d\n\n", station.udp_adj[ix], ix);

									strcpy(name, station.udp_adj_name[ix]);
									printf("can our neighbour name be copeid?: %s\n", name);
									int res = destination_handle(timeinfo, name);
									///return 1 but not guartee that there will be a time
									if (res == 1)
									{
										if (strstr(station.response, name) != NULL)
										{
											printf("--we can send request packet to our %s UDP: %d by:\n----%s\n", name, station.udp_adj[ix], station.response);
											char *saveptr;
											strcpy(packet.response, station.response);
											char *time = strtok_r(station.response, ",", &saveptr);
											time = strtok_r(NULL, ",", &saveptr);
											time = strtok_r(NULL, ",", &saveptr);
											time = strtok_r(NULL, ",", &saveptr);

											struct tm tm_time; //time in timetable
											memset(&tm_time, 0, sizeof(struct tm));
											strptime(time, "%H:%M", &tm_time);
											tm_time.tm_year = timeinfo.tm_year;
											tm_time.tm_mon = timeinfo.tm_mon;
											tm_time.tm_mday = timeinfo.tm_mday;
											printf("time->%d %d:%d\n", tm_time.tm_year, tm_time.tm_hour, tm_time.tm_min);
											packet.time_request = tm_time;

											int create = create_addr(station.udp_adj[ix], 3); //type 3 create UDP address to send to
											if (create == -1)
											{
												printf("CAnnot fucking create ADDress!!!\n");
											}

											int sen = sendto(station.udp_socket, &packet, sizeof(packet), 0, (struct sockaddr *)&address3, sizeof(address3));
											if (sen < 0)
											{
												printf("Holy Crap, Cant send REQUEST\n");
											}
											else
											{
												printf("Yeah we sent for REQUEST\n");
											}
										} //we want to ask our neighbour but it too late
									}	  //end of res==1 in asking UDP
								}		  //end for
							}			  //end of if resp==0
										  /*#################################^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^oooooooooooooooooooooooooooooo###########################################*/
						}				  //end if at checking token2
						else
						{
							printf("*******123***********RESPONE:Cant understand your REQUEST\n");
							char http_header[2048] = "HTTP/1.1 400 Bad Request\nContent-type: text/html\r\n\n";
							strcat(http_header, "Server cannot understand");
							send(sd, http_header, strlen(http_header), 0);
							close(sd);
							client_socket[i] = 0;
						} //end else: respone to TCP client that Sever don't understand request
					}	  //end if of finding GET
						  /****************************************************************************************************/
					else
					{
						//printf("*************************RESPONE:Cant understand your REQUEST\n");
						char http_header[2048] = "HTTP/1.1 400 Bad Request\nContent-type: text/html\r\n\n";
						strcat(http_header, "Server cannot understand other REQUEST but GET");
						send(sd, http_header, strlen(http_header), 0);
						close(sd);
						client_socket[i1] = 0;
					} //end else: respone to TCP client that Sever don't understand request     	/*******************************************************************************************/

					//you only want to handle request that does not have destination, not sending anything
					FILE *html_data;
					html_data = fopen("index.html", "r");

					char res_data[1024];
					fgets(res_data, 1024, html_data);
					
					//char http_header[2048] = "HTTP/1.1 200 OK\nContent-type: text/html\r\n\n";
					//strcat(http_header, res_data);
					//send(sd , http_header , strlen(http_header) , 0 );
					//close( sd );
					//client_socket[i] = 0;
				}
			}
		}
	}

	return 0;
}
