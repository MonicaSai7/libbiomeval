/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdlib>
#include <iostream>

#include <be_time.h>

using namespace std;
using namespace BiometricEvaluation;

int main(int argc, char *argv[])
{
	cout << "Current time is [" << Time::getCurrentTime() << "]" << endl;
	cout << "Current date is [" << Time::getCurrentDate() << "]" << endl;
	cout << "Current date and time is [" << Time::getCurrentDateAndTime() <<
	    "]" << endl;
	cout << "Current timezone is [" <<
	    Time::getCurrentCalendarInformation("%Z") << "]" << endl;

	return (EXIT_SUCCESS);
}
