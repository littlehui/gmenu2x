// SPDX-License-Identifier: GPL-2.0

#include "bottombar.h"
#include "clock.h"
#include "gmenu2x.h"
#include "compat-filesystem.h"

#include <tuple>

BottomBar::BottomBar(GMenu2X& gmenu2x)
	: gmenu2x(gmenu2x), battery(gmenu2x), freeMiB(-1), totalMiB(-1),
	  cpufreq(0), hours(99), minutes(99), timeIs24(true), manualEnabled(false)
{
	setSize(0, gmenu2x.skinConfInt["bottomBarHeight"]);
	setContainer(LAY_FLEX | LAY_ROW | LAY_START);
	setBehave(LAY_HFILL);

	margin = gmenu2x.skinConfInt["bottomBarHeight"] / 2;

	auto sd = std::make_shared<LayoutSurface>(gmenu2x.sc.skinRes("imgs/sd.png"));
	sd->setMargins(margin, 0, 0, 0);
	addChild(sd);

	sd_text.reset();
	updateDiskFreeText();

	auto cpu_icon = std::make_shared<LayoutSurface>(gmenu2x.sc.skinRes("imgs/cpu.png"));
	cpu_icon->setMargins(margin, 0, 0, 0);
	cpu = cpu_icon;
	addChild(cpu);

	cpu_text.reset();

	manual = std::make_shared<LayoutSurface>(gmenu2x.sc.skinRes("imgs/manual.png"));
	manual->setMargins(margin, 0, 0, 0);
	addChild(manual);

	auto spacer = std::make_shared<LayoutItem>();
	spacer->setBehave(LAY_HFILL);
	addChild(spacer);

	updateTimeText();

	updateBatteryIcon();

	/* These are hidden by default */
	cpu->hide();
	manual->hide();
}

void BottomBar::update()
{
	updateDiskFreeText();
	updateTimeText();
	updateBatteryIcon();
}

void BottomBar::enableManualIcon(bool enable)
{
	manual->setVisible(enable);
}

void BottomBar::showCpuFreq(unsigned long mhz)
{
#ifdef ENABLE_CPUFREQ
	if (!mhz && !!cpufreq) {
		cpu->hide();
		cpu_text->hide();
	} else if (cpufreq != mhz) {
		std::ostringstream ss;
		ss << mhz << "MHz";

		auto text_s = gmenu2x.font->render(ss.str());
		auto new_cpu_text = std::make_shared<LayoutSurface>(text_s);

		if (cpu_text) {
			addSibling(cpu_text, new_cpu_text);
			removeChild(cpu_text);
		} else {
			addSibling(cpu, new_cpu_text);
		}

		cpu_text = new_cpu_text;

		cpu->show();
		cpu_text->show();
	}
#endif

	cpufreq = mhz;
}

void BottomBar::updateBattery()
{
	battery.update();
}

bool BottomBar::updateDiskFree()
{
	unsigned long free = 0, total = 0;
	std::error_code ec;
	auto space = compat::filesystem::space(GMenu2X::getHome(), ec);

	if (!ec) {
		free = space.free / (1024 * 1024);
		total = space.capacity / (1024 * 1024);
	}

	if (free == freeMiB && total == totalMiB)
		return false;

	freeMiB = free;
	totalMiB = total;
	return true;
}

void BottomBar::updateDiskFreeText()
{
	if (!updateDiskFree())
		return;

	std::ostringstream ss;
	if (totalMiB >= 10000) {
		ss << (freeMiB / 1024) << "." << ((freeMiB % 1024) * 10) / 1024 << "/"
		   << (totalMiB / 1024) << "." << ((totalMiB % 1024) * 10) / 1024 << "GiB";
	} else {
		ss << freeMiB << "/" << totalMiB << "MiB";
	}

	auto text_s = gmenu2x.font->render(ss.str());
	auto new_sd_text = std::make_shared<LayoutSurface>(text_s);

	if (sd_text) {
		addSibling(sd_text, new_sd_text);
		removeChild(sd_text);
	} else {
		addChild(new_sd_text);
	}

	sd_text = new_sd_text;
}

bool BottomBar::updateTime()
{
#ifdef ENABLE_CLOCK
	unsigned new_hours, new_minutes;
	Clock clock;

	std::tie(new_hours, new_minutes) = clock.getCurrentTime();
    new_hours += 8;
	if (new_hours != hours || new_minutes != minutes) {
		hours = new_hours;
		minutes = new_minutes;
		return true;
	}
#endif

	return false;
}

void BottomBar::updateTimeText()
{
	if (!updateTime())
		return;

	std::ostringstream ss;
	ss.width(2);
	ss.fill('0');

	/* C++ streams are beyond stupid */
	ss << std::setfill('0') << std::setw(2) << (timeIs24 ? hours : hours % 12);
	ss << ":" << std::setfill('0') << std::setw(2) << minutes;
	if (!timeIs24)
		ss << (hours >= 12 ? "pm" : "am");

	auto text_s = gmenu2x.font->render(ss.str());
	auto new_clock_text = std::make_shared<LayoutSurface>(text_s);

	new_clock_text->setMargins(margin, 0, margin, 0);

	if (clock_text) {
		addSibling(clock_text, new_clock_text);
		removeChild(clock_text);
	} else {
		addChild(new_clock_text);
	}

	clock_text = new_clock_text;
}

void BottomBar::updateBatteryIcon()
{
	auto icon = battery.getIcon();

	if (battery_icon && battery_icon->getSurface() == icon)
		return;

	auto icon_item = std::make_shared<LayoutSurface>(icon);

	icon_item->setMargins(margin, 0, 0, 0);

	if (battery_icon) {
		addSibling(battery_icon, icon_item);
		removeChild(battery_icon);
	} else {
		addChild(icon_item);
	}

	battery_icon = icon_item;
}
