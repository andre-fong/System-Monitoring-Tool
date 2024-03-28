/*
	CSCB09: Assignment 1
	Andre Fong
*/

#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<sys/resource.h>
#include<sys/utsname.h>
#include<sys/sysinfo.h>
#include<sys/types.h>
#include<utmp.h>
#include<unistd.h>

#define GiB 1073741824

/*
 * Function: extractFlagValue
 * ----------------------------
 * Extracts a positive numerical value from a string 
 * with the form: --<flagName>=<positiveNumericalValue>
 * 
 * flag: string with above flag format
 * flagName: name of flag to validate
 *
 * returns: the positive numerical value from the flag if flag is valid
 *          <0 on error (if flag is wrong format)
 */
int extractFlagValue(char *flag, char *flagName) {
	if (flag == NULL || flagName == NULL) return -1;
	
	// Ensure first two chars of flag is "--"
	char *tr = flag;
	for (int i = 0; i < 2; i++) {
		if (flag[i] != '-' || flag[i] == '\0') return -2;
		tr++;
	}
	
	// Find first occurrence of flagName and make sure it's right after the flag delimiter
	char *firstOccur;
	firstOccur = strstr(tr, flagName);
	
	// No occurrence of flagName
	if (firstOccur == NULL) return -3;
	
	// flagName not located immediately after "--"
	if (firstOccur != tr) return -4;
	
	// flagName is correct, ensure '=' located right after flagName
	firstOccur += strlen(flagName);
	if (*firstOccur != '=') return -5;
	firstOccur++;
	
	// Convert value after '=' to long int, error if anything after '=' cannot be converted
	char *leftover;
	long value = strtol(firstOccur, &leftover, 10);
	
	if (*leftover != '\0') return -6;
	
	return value;
}

/*
 * Function: printSectionLine
 * ----------------------------
 * Prints 40 '-' characters in a line to divide printed sections
 * 
 * returns: nothing
 */
void printSectionLine() {
	printf("---------------------------------------\n");
}

/*
 * Function: formatToTwoDigits
 * ----------------------------
 * Takes a number and appends a 0 to the beginning 
 * if the number is one digit
 *
 * num: Positive integer (0 <= num)
 * strAddress: Address of string to copy to, must have allocated space for at least 3 chars
 *
 * returns: the number formatted to be 2 digits long as a string
 */
void formatToTwoDigits(int num, char *strAddress) {
	if (num < 10) sprintf(strAddress, "0%d", num);
	else sprintf(strAddress, "%d", num);
}

// Struct for storing samples of memory data in linked list
typedef struct MemoryNode {
	float physUsed;	// GB
	float physTot;	// GB
	float virtUsed;	// GB
	float virtTot;	// GB
	struct MemoryNode *next;
} MemoryNode;
// Linked list operations
MemoryNode *newMNode(float physUsed, float physTot, float virtUsed, float virtTot) {
	MemoryNode *new = malloc(1 * sizeof(MemoryNode));
	if (new == NULL) fprintf(stderr, "Error allocating memory for MemoryNode\n.");
	new->physUsed = physUsed;
	new->physTot = physTot;
	new->virtUsed = virtUsed;
	new->virtTot = virtTot;
	new->next = NULL;
	return new;
}
void insertMAtTail(MemoryNode *head, MemoryNode *new) {
	MemoryNode *tr = head;
	while (tr->next != NULL) tr = tr->next;
	tr->next = new;
}
void printMList(MemoryNode *head, bool sequential, bool graphics) {
	MemoryNode *tr = head;
	MemoryNode *pre = NULL;
	float memDiff = 0;
	while (tr != NULL) {
		if (pre != NULL) {
			memDiff = tr->virtUsed - pre->virtUsed;
		}
		
		if (sequential) {
			if (tr->next == NULL) printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", tr->physUsed, tr->physTot, tr->virtUsed, tr->virtTot);
			// else printf("\n");
		}
		else printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", tr->physUsed, tr->physTot, tr->virtUsed, tr->virtTot);
		
		if ((graphics && !sequential) || (graphics && sequential && tr->next == NULL)) {
			printf("     |");
			if (tr == head) printf("o %.2f", memDiff);
			else if (memDiff == 0) printf("* %.2f", memDiff);
			else if (memDiff < 0) {
				int least = ((int)(-memDiff / 0.01) - 1 < 20) ? (int)(-memDiff / 0.01) - 1 : 20;
				for (int b = 0; b < least; b++) printf(":");
				if ((int)(-memDiff / 0.01) - 1 >= 20) printf("...");
				printf("@ %.2f", memDiff);
			}
			else {
				int least = ((int)(memDiff / 0.01) - 1 < 20) ? (int)(memDiff / 0.01) - 1 : 20;
				for (int c = 0; c < least; c++) printf("#");
				if ((int)(memDiff / 0.01) - 1 >= 20) printf("...");
				printf("* %.2f", memDiff);
			}
			printf(" (%.2f)", tr->virtUsed);
		}
		
		printf("\n");
		pre = tr;
		tr = tr->next;
	}
}
void deleteMList(MemoryNode *head) {
	MemoryNode *pre = head;
	MemoryNode *tr = NULL;
	while (pre != NULL) {
		tr = pre->next;
		free(pre);
		pre = tr;
	}
}

// Struct for storing samples of cpu usage in a linked list
typedef struct CpuUseNode {
	float cpuUse;
	struct CpuUseNode *next;
} CpuUseNode;
// Linked list operations
CpuUseNode *newCNode(float cpuUse) {
	CpuUseNode *new = malloc(1 * sizeof(CpuUseNode));
	if (new == NULL) fprintf(stderr, "Error allocating memory for CpuUseNode\n.");
	new->cpuUse = cpuUse;
	new->next = NULL;
	return new;
}
void insertCAtTail(CpuUseNode *head, CpuUseNode *new) {
	CpuUseNode *tr = head;
	while (tr->next != NULL) tr = tr->next;
	tr->next = new;
}
float getLastCpuUse(CpuUseNode *head) {
	CpuUseNode *tr = head;
	while (tr->next != NULL) tr = tr->next;
	return tr->cpuUse;
}
void printCList(CpuUseNode *head, bool sequential) {
	CpuUseNode *tr = head;
	int lineCounter = 0;
	while (tr != NULL) {
		printf("\033[K\t");
		if ((sequential && tr->next == NULL) || !sequential) {
			printf("|||");
			for (int a = 0; a < (int)(tr->cpuUse); a++) printf("|");
			printf(" %.2f\n", tr->cpuUse);
		}
		else printf("\n");
		
		lineCounter++;
		tr = tr->next;
	}
	
	printf("\033[%dA", lineCounter);
}
void deleteCList(CpuUseNode *head) {
	CpuUseNode *pre = head;
	CpuUseNode *tr = NULL;
	while (pre != NULL) {
		tr = pre->next;
		free(pre);
		pre = tr;
	}
}

int main(int argc, char **argv) {
	// Default program arguments
	bool system = false, user = false, graphics = false, sequential = false;
	int samples = 10, delay = 1;
	
	bool sawSamplesPosArg = false;
	bool sawAllPosArgs = false;
	bool brokePosArg = false;
	bool sawFlaggedSamples = false;
	bool sawFlaggedDelay = false;
	
	// Parse through CLAs
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--system", 8) == 0) {
			system = true;
			brokePosArg = true;
		}
		else if (strncmp(argv[i], "--user", 6) == 0) {
			user = true;
			brokePosArg = true;
		}
		else if (strncmp(argv[i], "--graphics", 10) == 0) {
			graphics = true;
			brokePosArg = true;
		}
		else if (strncmp(argv[i], "--sequential", 12) == 0) {
			sequential = true;
			brokePosArg = true;
		}
		else {
			char *leftover;
			long numArg = strtol(argv[i], &leftover, 10);
			
			// If arg is positional arg (integer)
			if (leftover[0] == '\0') {
				// More than 2 positional arguments
				if (sawAllPosArgs) {
					fprintf(stderr, "args: only a maximum of two positional arguments can be provided\n");
					return 1;
				}
				
				// Another flag separates the positional arguments
				if (brokePosArg && sawSamplesPosArg) {
					fprintf(stderr, "args: positional arguments must be contiguous\n");
					return 1;
				}
				// Second positional argument
				else if (sawSamplesPosArg) {
					if (!sawFlaggedDelay) delay = numArg;
					sawAllPosArgs = true;
				}
				// First positional argument
				else {
					if (!sawFlaggedSamples) samples = numArg;
					sawSamplesPosArg = true;
					brokePosArg = false;
				}
			}
			
			// Test if arg is --samples=N or --tdelay=T
			else {
				brokePosArg = true;
				
				int samplesRes = extractFlagValue(argv[i], "samples");
				int delayRes = extractFlagValue(argv[i], "tdelay");
				
				// No remaining possible arguments that match argv[i]
				if (samplesRes < 0 && delayRes < 0) {
					fprintf(stderr, "args: unsupported argument: \"%s\"\n", argv[i]);
					return 1;
				}
				
				if (samplesRes >= 0) {
					samples = samplesRes;
					sawFlaggedSamples = true;
				}
				else if (delayRes >= 0) {
					delay = delayRes;
					sawFlaggedDelay = true;
				}
				else {
					fprintf(stderr, "args: something went wrong");
					return 1;
				}
			}
		}

	}
	
	// Allocate space for system statistics
	MemoryNode *memoryHead = NULL;
	CpuUseNode *cpuHead = NULL;
	
	// Declare vars for memory usage report
	struct rusage usage;
	long curProgMem;
	float physTot, physUsed, virtTot, virtUsed;
	
	// Get initial computer uptime
	struct sysinfo systemInfo;
	sysinfo(&systemInfo);
	long uptime = systemInfo.uptime;
	
	// Get users logged in to machine
	FILE *usersFile;
	usersFile = fopen("/var/run/utmp", "rb");
	if (usersFile == NULL) {
		fprintf(stderr, "error: /var/run/utmp could not be opened\n");
		return 1;
	}
	struct utmp userInfo;
	
	// Get initial CPU info
	FILE *cpuInfo;
	cpuInfo = fopen("/proc/cpuinfo", "r");
	if (cpuInfo == NULL) {
		fprintf(stderr, "error: /proc/cpuinfo could not be opened\n");
		return 1;
	}
	char line[100];
	int cores = -1;
	while (fgets(line, 100, cpuInfo) != NULL && cores == -1) {
		if (strstr(line, "cpu cores") != NULL) {
			sscanf(line, "%*s %*s : %d", &cores);
		}
	}
	if (cores == -1) {
		fprintf(stderr, "error: Could not find # of cpu cores\n");
		return 1;
	}
	
	// Get initial CPU usage
	long initialTotTime, newTotTime, initialTimeIdle, newTimeIdle;
	long userT, niceT, systemT, idleT, iowaitT, irqT, softirqT;
	float cpuUsage;
	FILE *stats;
	
	int sleepTime = (delay > 1) ? delay - 1 : 1;	// delay for gathering change in CPU stats (min. 1s)
	
	// Program start
	for (int i = 0; i < samples; i++) {
		/* SAMPLE SYSTEM DATA */
		// Memory usage
		getrusage(RUSAGE_SELF, &usage);
		curProgMem = usage.ru_maxrss;
		
		sysinfo(&systemInfo);
		physTot = (float)systemInfo.totalram / (float)GiB;
		physUsed = physTot - ((float)systemInfo.freeram / (float)GiB);
		virtTot = ((float)systemInfo.totalswap / (float)GiB) + physTot;
		virtUsed = ((float)systemInfo.totalswap / (float)GiB) - ((float)systemInfo.freeswap / (float)GiB) + physUsed;
		// Write new memory sample on linked list
		if (memoryHead == NULL) memoryHead = newMNode(physUsed, physTot, virtUsed, virtTot);
		else {
			MemoryNode *new = newMNode(physUsed, physTot, virtUsed, virtTot);
			insertMAtTail(memoryHead, new);
		}
		
		/* CLEAR TERMINAL AND MOVE CURSOR TO TOP LEFT */
		if (!sequential) 
			printf("\033[2J\033[H");
		
		/* PRINT DATA (without CPU) */
		// Running parameters
		if (sequential) printf(">>> iteration %d\n", i);
		else printf("Nbr of samples: %d -- every %d secs\n", samples, delay);
		printf(" Memory usage: %ld kilobytes\n", curProgMem);	// program memory usage always printed
		printSectionLine();
		
		// Memory usage
		if (!user || system) {
			printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
			printMList(memoryHead, sequential, graphics);
			for (int j = 0; j < samples - i - 1; j++) printf("\n");		// fill blank space for memory section
			printSectionLine();
		}
		
		// User usage
		if (user || !system) printf("### Sessions/users ###\n");
		// Reset file pointer to beginning of file
		if (fseek(usersFile, SEEK_SET, SEEK_SET) != 0) {
			fprintf(stderr, "error: fseek was not successful\n");
			return 1;
		}
		// Parse through list of users and print logged in users
		while (fread(&userInfo, sizeof(struct utmp), 1, usersFile) == 1) {
			if (userInfo.ut_type != USER_PROCESS) continue;
			// printf("user: |%d| |%d| |%s| |%s| |%s| |%s| |%d| |%d|\n", userInfo.ut_type, userInfo.ut_pid, userInfo.ut_line, userInfo.ut_id, userInfo.ut_user, userInfo.ut_host, userInfo.ut_session, userInfo.ut_addr_v6[0]);
			
			char userDetails[UT_HOSTSIZE];
			// TTY
			if (strstr(userInfo.ut_line, "pts") == NULL) {
				strncpy(userDetails, userInfo.ut_host, 15);
				userDetails[15] = '\0';
			}
			// PTS
			else {
				// no IPV4, display tmux
				if (userInfo.ut_host[0] == '\0') {
					sprintf(userDetails, "tmux(%d).%%0", userInfo.ut_session);
				}
				// display IPV4
				else {
					strncpy(userDetails, userInfo.ut_host, 15);
					userDetails[15] = '\0';
				}
			}
			
			if (user || !system) printf(" %s\t%s (%s)\n", userInfo.ut_user, userInfo.ut_line, userDetails);
		}
		if (user || !system) printSectionLine();
		
		// CPU Usage
		stats = fopen("/proc/stat", "r");
		if (stats == NULL) {
			fprintf(stderr, "error: /proc/stat could not be opened\n");
			return 1;
		}
		if (!user || system) printf("Number of cores: %d\n", cores);
		if (fscanf(stats, "%*s %ld %ld %ld %ld %ld %ld %ld", &userT, &niceT, &systemT, &idleT, &iowaitT, &irqT, &softirqT) != 7) {
			fprintf(stderr, "warn: Some fields are missing when getting cpu stats\n");
		}
		
		if (cpuHead != NULL && (!user || system)) {
			printf(" total cpu use = %.2f%%\n", getLastCpuUse(cpuHead));
			if (graphics) printCList(cpuHead, sequential);
		}
		
		fseek(stats, SEEK_SET, SEEK_SET);
		initialTimeIdle = idleT;
		initialTotTime = userT + niceT + systemT + idleT + iowaitT + irqT + softirqT;

		// Wait and then sample CPU stats again
		sleep(sleepTime);	// delay min of 1 sec to wait for /proc/stat to update
		if (fscanf(stats, "%*s %ld %ld %ld %ld %ld %ld %ld", &userT, &niceT, &systemT, &idleT, &iowaitT, &irqT, &softirqT) != 7) {
			fprintf(stderr, "warn: Some fields are missing when getting cpu stats\n");
		}
		fseek(stats, SEEK_SET, SEEK_SET);
		newTimeIdle = idleT;
		newTotTime = userT + niceT + systemT + idleT + iowaitT + irqT + softirqT;
		
		if (newTimeIdle - initialTimeIdle == 0 || newTotTime - initialTotTime == 0) {
			cpuUsage = 0;
		}
		else cpuUsage = 100 * (double)((newTotTime - newTimeIdle) - (initialTotTime - initialTimeIdle)) / (double)(newTotTime - initialTotTime);

		// Write new cpu usage stat to linked list
		if (cpuHead == NULL) cpuHead = newCNode(cpuUsage);
		else {
			CpuUseNode *new = newCNode(cpuUsage);
			insertCAtTail(cpuHead, new);
		}
		// Update cpu usage without clearing screen
		if (!user || system) {
			if (i > 0) printf("\033[1A\033[K");	// to refresh cpu usage without clearing entire screen
			printf(" total cpu use = %.2f%%\n", getLastCpuUse(cpuHead));
			if (graphics) printCList(cpuHead, sequential);
		}
		
		int statsErr = fclose(stats);
		if (statsErr != 0) {
			fprintf(stderr, "error: /proc/stat could not be closed\n");
			return 1;
		}
		
		/* DELAY */
		sleep((delay - sleepTime >= 0) ? delay - sleepTime : 0);	// delay remaining time
	}
	
	if (graphics) printf("\033[%dB", samples);	// Realign output pointer
	
	/* GET SYSTEM INFORMATION */
	struct utsname kernalInfo;
	uname(&kernalInfo);
	char *systemName = kernalInfo.sysname;
	char *machineName = kernalInfo.nodename;
	char *version = kernalInfo.version;
	char *release = kernalInfo.release;
	char *architecture = kernalInfo.machine;
	
	// Create time string
	int daysUptime = floor(uptime / 86400);
	int hoursUptime = floor((uptime % 86400) / 3600);
	int minsUptime = floor((uptime % 3600) / 60);
	int secsUptime = uptime % 60;
	char totalHoursUpStr[256];
	char hoursUpStr[3];
	char minsUpStr[3];
	char secsUpStr[3];
	formatToTwoDigits(floor(uptime / 3600), totalHoursUpStr);
	formatToTwoDigits(hoursUptime, hoursUpStr);
	formatToTwoDigits(minsUptime, minsUpStr);
	formatToTwoDigits(secsUptime, secsUpStr);
	
	// Print system information
	printSectionLine();
	printf("### System Information ###\n");
	printf(" System Name = %s\n", systemName);
	printf(" Machine Name = %s\n", machineName);
	printf(" Version = %s\n", version);
	printf(" Release = %s\n", release);
	printf(" Architecture = %s\n", architecture);
	printf(" System running since last reboot: %d days %s:%s:%s (%s:%s:%s)\n", daysUptime, hoursUpStr, minsUpStr, secsUpStr, totalHoursUpStr, minsUpStr, secsUpStr);
	printSectionLine();
	
	// Free allocated memory
	deleteMList(memoryHead);
	deleteCList(cpuHead);
	
	// Close /var/run/utmp
	int error = fclose(usersFile);
	if (error != 0) {
		fprintf(stderr, "error: /var/run/utmp could not be closed\n");
		return 1;
	}
	// Close /proc/cpuinfo
	error = fclose(cpuInfo);
	if (error != 0) {
		fprintf(stderr, "error: /proc/cpuinfo could not be closed\n");
		return 1;
	}
	
	return 0;
}
