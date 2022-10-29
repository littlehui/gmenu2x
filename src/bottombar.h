/* SPDX-License-Identifier: GPL-2.0 */

#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include "battery.h"
#include "layer.h"

class BottomBar : public Layer {
public:
	BottomBar(GMenu2X& gmenu2x);
	~BottomBar() {}

	virtual void paint(Surface& s) {}
	virtual bool handleButtonPress(InputManager::Button button) { return false; }

	void update();

	void enableManualIcon(bool enable);
	void showCpuFreq(unsigned long mhz = 0);
	void updateBattery();

private:
	GMenu2X& gmenu2x;
	Battery battery;
	unsigned long freeMiB, totalMiB, cpufreq;
	unsigned int hours, minutes, margin;
	bool timeIs24, manualEnabled, cpuFreqEnabled;

	std::shared_ptr<LayoutSurface> sd_text, clock_text,
		battery_icon, cpu, cpu_text, manual;

	bool updateDiskFree();
	bool updateTime();

	void updateDiskFreeText();
	void updateTimeText();
	void updateBatteryIcon();
};

#endif /* BOTTOMBAR_H */
