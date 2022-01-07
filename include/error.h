#ifndef ERROR_H
#define ERROR_H

const char *intercal[] = {
    "E017 DO YOU EXPECT ME TO FIGURE THIS OUT?",
    "E079 PROGRAMMER IS INSUFFICIENTLY POLITE",
    "E099 PROGRAMMER IS OVERLY POLITE",
    "E111 COMMUNIST PLOT DETECTED, COMPILER IS SUICIDING",
    "E123 PROGRAM HAS DISAPPEARED INTO THE BLACK LAGOON",
    "E127 SAYING ’ABRACADABRA’ WITHOUT A MAGIC WAND WON’T DO YOU ANY GOOD",
    "E129 PROGRAM HAS GOTTEN LOST",
    "E139 I WASN’T PLANNING TO GO THERE ANYWAY",
    "E182 YOU MUST LIKE THIS LABEL A LOT!",
    "E197 SO! 65535 LABELS AREN’T ENOUGH FOR YOU?",
    "E200 NOTHING VENTURED, NOTHING GAINED",
    "E222 BUMMER, DUDE!",
    "E240 ERROR HANDLER PRINTED SNIDE REMARK",
    "E241 VARIABLES MAY NOT BE STORED IN WEST HYPERSPACE",
    "E252 I’VE FORGOTTEN WHAT I WAS ABOUT TO SAY",
    "E256 THAT’S TOO HARD FOR MY TINY BRAIN",
    "E275 DON’T BYTE OFF MORE THAN YOU CAN CHEW",
    "E277 YOU CAN ONLY DISTORT THE LAWS OF MATHEMATICS SO FAR",
    "E281 THAT MUCH QUOTATION AMOUNTS TO PLAGIARISM",
    "E333 YOU CAN’T HAVE EVERYTHING, WHERE WOULD YOU PUT IT?",
    "E345 THAT’S TOO COMPLEX FOR ME TO GRASP",
    "E404 I’M ALL OUT OF CHOICES!",
    "E405 PROGRAM REJECTED FOR MENTAL HEALTH REASONS",
    "E436 THROW STICK BEFORE RETRIEVING!",
    "E444 IT CAME FROM BEYOND SPACE",
    "E533 YOU WANT MAYBE WE SHOULD IMPLEMENT 64-BIT VARIABLES?",
    "E553 BETTER LATE THAN NEVER",
    "E555 FLOW DIAGRAM IS EXCESSIVELY CONNECTED",
    "E562 I DO NOT COMPUTE",
    "E579 WHAT BASE AND/OR LANGUAGE INCLUDES string?",
    "E621 ERROR TYPE 621 ENCOUNTERED",
    "E632 THE NEXT STACK RUPTURES. ALL DIE. OH, THE EMBARRASSMENT!",
    "E633 PROGRAM FELL OFF THE EDGE",
    "E652 HOW DARE YOU INSULT ME!",
    "E666 COMPILER HAS INDIGESTION",
    "E774 RANDOM COMPILER BUG",
    "E777 A SOURCE IS A SOURCE, OF COURSE, OF COURSE",
    "E778 UNEXPLAINED COMPILER BUG",
    "E810 ARE ONE-CHARACTER COMMANDS TOO SHORT FOR YOU?",
    "E811 PROGRAM IS TOO BADLY BROKEN TO RUN",
    "E888 I HAVE NO FILE AND I MUST SCREAM",
    "E899 HELLO? CAN ANYONE GIVE ME A HAND HERE?",
    "E990 FLAG ETIQUETTE FAILURE BAD SCOUT NO BISCUIT",
    "E991 YOU HAVE TOO MUCH ROPE TO HANG YOURSELF",
    "E993 I GAVE UP LONG AGO",
    "E994 NOCTURNAL EMISSION, PLEASE LAUNDER SHEETS IMMEDIATELY",
    "E995 DO YOU REALLY EXPECT ME TO HAVE IMPLEMENTED THAT?",
    "E997 ILLEGAL POSSESSION OF A CONTROLLED UNARY OPERATOR",
    "E998 EXCUSE ME, YOU MUST HAVE ME CONFUSED WITH SOME OTHER COMPILER",
    "E999 NO SKELETON IN MY CLOSET, WOE IS ME!",
    "W016 DON’T TYPE THAT SO HASTILY",
    "W018 THAT WAS MEANT TO BE A JOKE",
    "W112 THAT RELIES ON THE NEW WORLD ORDER",
    "W128 SYSLIB IS OPTIMIZED FOR OBUSCATION",
    "W276 YOU CAN’T EXPECT ME TO CHECK BACK THAT FAR",
    "W239 WARNING HANDLER PRINTED SNIDE REMARK",
    "W278 FROM A CONTRADICTION, ANYTHING FOLLOWS",
    "W450 THE DOCUMENTOR IS NOT ALWAYS RIGHT",
    "W534 KEEP LOOKING AT THE TOP BIT",
    "W622 WARNING TYPE 622 ENCOUNTERED"
};

#define error_iprint() fprintf(stderr, "%s\n", intercal[randrng(0, nelem(intercal))])

#endif