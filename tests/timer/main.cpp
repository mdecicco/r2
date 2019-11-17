#include <r2/utilities/timer.h>
using namespace r2;

int main(int argc, char** argv) {
	timer time;
	timer pauseTmr;
	timer logTmr;

	time.start();
	pauseTmr.start();
	logTmr.start();
	while(true) {
		if (logTmr.elapsed() >= 0.1f) {
			logTmr.reset();
			logTmr.start();
			if (!time.stopped()) printf("%0.2f\n", time.elapsed());
		}

		if (pauseTmr.elapsed() >= 1.0f) {
			pauseTmr.reset();
			pauseTmr.start();
			if (time.stopped()) {
				printf("resuming\n");
				time.start();
			} else {
				printf("pausing\n");
				time.pause();
			}
		}
	}

	return 0;
}
