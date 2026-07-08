/**************************************************************************************
 *DisplaySettings.c
 *
 *This module controls the output of the program. Has one public function for initialization,
 *one private function for settings up the display, and a task that updates the display
 *every 250ms if an input has been changed.
 *
 *Created on: Mar 12, 2025
 *Author: Trey McCabe
 ***************************************************************************************/

#ifndef DISPLAYSETTINGS_H_
#define DISPLAYSETTINGS_H_

/*************************************************************************
 *Public Function ProtoTypes
 *************************************************************************/

/**************************************************************************
 *void displaySettingsInit(void)
 *Initialization function that create the task, and displays the default
 *system settings.
 **************************************************************************/
void displaySettingsInit(void);

#endif /* DISPLAYSETTINGS_H */
