/* In this file we describe esq. sequense for coloring alarm
for different type of printer. 
See http://hpcc920.external.hp.com/cposupport/eschome.html 
for the other HP printer */
/* ------------- Mono printer ---------------------------------- */ 

char colorStartNoMono[] ={{0}};
 int colorStartNoMonoLen=0;

char colorStartMinorMono[] ={{0}};
 int colorStartMinorMonoLen=0;

char colorStartMajorMono[]={{27},{'['},{'1'},{'m'},{0}};  /*BOLD letters*/
 int colorStartMajorMonoLen=5;

char colorStartInvalidMono[]={{27},{'['},{'1'},{'m'},{0}};/*BOLD letters*/
 int colorStartInvalidMonoLen=5;

char colorEndMono[]={{27},{'['},{'0'},{'m'},{0}};
int colorEndMonoLen=5;

/* ------------- HP DeskJet 1200C ---------------------------------- */
 
char colorStartNoHpColor[] ={{0}};
 int colorStartNoHpColorLen=0;

char colorStartMinorHpColor[] ={{27},{'&'},{'v'},{'3'},{'S'},{0}}; 
 int colorStartMinorHpColorLen=6;              /*YELLOW letters*/
                               
char colorStartMajorHpColor[]={{27},{'&'},{'v'},{'1'},{'S'},{0}};
 int colorStartMajorHpColorLen=6;                 /*RED letters*/

char colorStartInvalidHpColor[]={{27},{'&'},{'v'},{'5'},{'S'},{0}};
 int colorStartInvalidHpColorLen=6;             /*MAGNETA letters*/

char colorEndHpColor[]={{27},{'E'},{0}};
 int colorEndHpColorLen=3;


