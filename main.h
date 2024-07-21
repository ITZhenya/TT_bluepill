#ifndef _MAIN_H
#define _MAIN_H

#define PROGRAM_BOOTLOADER      // In Linker select default icf-file
//#define PROGRAM_VER1          // In Linker select icf-file: $PROJ_DIR$\Project.icf
//#define PROGRAM_VER2          // In Linker select icf-file: $PROJ_DIR$\Project.icf

/* systick event function */
void systick_event(void);

#endif /* _MAIN_H */