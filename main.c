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
	printf("----------------------------------------\n");
}

/*
 * Function: formatToTwoDigits
 * ----------------------------
 * Takes a number (max two digits) and appends a 0 to the beginning 
 * if the number is one digit
 *
 * num: Positive integer with at most 2 digits (0 <= num < 100)
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
MemoryNode *newNode(float physUsed, float physTot, float virtUsed, float virtTot) {
	MemoryNode *new = malloc(1 * sizeof(MemoryNode));
	new->physUsed = physUsed;
	new->physTot = physTot;
	new->virtUsed = virtUsed;
	new->virtTot = virtTot;
	new->next = NULL;
	return new;
}
void insertAtTail(MemoryNode *head, MemoryNode *new) {
	MemoryNode *tr = head;
	while (tr->next != NULL) tr = tr->next;
	tr->next = new;
}
void printList(MemoryNode *head) {
	MemoryNode *tr = head;
	while (tr != NULL) {
		printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\n", tr->physUsed, tr->physTot, tr->virtUsed, tr->virtTot);
		tr = tr->next;
	}
}
void deleteList(MemoryNode *head) {
	MemoryNode *pre = head;
	MemoryNode *tr = NULL;
	while (pre != NULL) {
		tr = pre->next;
		free(pre);
		pre = tr;
	}
}

int mainTEST(int argc, char **argv) {
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
	
	if (system && user) {
		fprintf(stderr, "args: only one of \"--system\" or \"--user\" can be provided as valid flags\n");
		return 1;
	}
	
	// Allocate space for system statistics
	MemoryNode *memoryHead = NULL;
	
	struct rusage usage;
	int rusageRet = getrusage(RUSAGE_SELF, &usage);
	long curProgMem = usage.ru_maxrss;
	
	struct sysinfo systemInfo;
	int sysinfoRet = sysinfo(&systemInfo);
	long uptime = systemInfo.uptime;
	
	// Program start
	for (int i = 0; i < samples; i++) {
		/* SAMPLE SYSTEM DATA */
		// Memory usage
		rusageRet = getrusage(RUSAGE_SELF, &usage);
		curProgMem = usage.ru_maxrss;
		
		sysinfoRet = sysinfo(&systemInfo);
		float physTot = (float)systemInfo.totalram / (float)GiB;
		float physUsed = physTot - ((float)systemInfo.freeram / (float)GiB);
		float virtTot = (float)systemInfo.totalswap / (float)GiB;
		float virtUsed = virtTot - ((float)systemInfo.freeswap / (float)GiB);
		if (memoryHead == NULL) memoryHead = newNode(physUsed, physTot, virtUsed, virtTot);
		else {
			MemoryNode *new = newNode(physUsed, physTot, virtUsed, virtTot);
			insertAtTail(memoryHead, new);
		}
		
		/* CLEAR TERMINAL AND MOVE CURSOR TO TOP LEFT */
		printf("\033[2J\033[H");
		
		/* PRINT DATA */
		// Running parameters
		printf("Nbr of samples: %d -- every %d secs\n", samples, delay);
		printf(" Memory usage: %ld kilobytes\n", curProgMem);
		printSectionLine();
		
		// Memory usage
		printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
		printList(memoryHead);
		
		// User usage
		
		// CPU Usage
		
		/* DELAY */
		sleep(delay);
	}
	
	/* GET SYSTEM INFORMATION */
	struct utsname kernalInfo;
	int utsRet = uname(&kernalInfo);
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
	char totalHoursUpStr[3];
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
	deleteList(memoryHead);
	
	// TODO: DELETE BELOW DEBUG CODE AFTER
	printf("\nDEBUG: sys: %d, user: %d, graphics: %d, seq: %d, samples: %d, delay: %d\n", system, user, graphics, sequential, samples, delay);
	return 0;
}

int main(int argc, char **argv) {
	return mainTEST(argc, argv);
}
