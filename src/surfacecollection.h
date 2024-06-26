/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo   *
 *   massimiliano.torromeo@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef SURFACECOLLECTION_H
#define SURFACECOLLECTION_H

#include <memory>
#include <string>
#include <unordered_map>

class GMenu2X;
class OffscreenSurface;
class Surface;

/**
Hash Map of surfaces that loads surfaces not already loaded and reuses already loaded ones.

	@author Massimiliano Torromeo <massimiliano.torromeo@gmail.com>
*/
class SurfaceCollection {
public:
	SurfaceCollection(GMenu2X *gmenu2x);
	~SurfaceCollection();

	void setSkin(const std::string &skin);
	std::string getSkinFilePath(const std::string &file, bool useDefault = true);
	std::string getSkinPath(const std::string &skin);

	void debug();

	std::shared_ptr<OffscreenSurface> addSkinRes(const std::string &path, bool useDefault = true);
	void     del(const std::string &path);
	void     clear();
	void     move(const std::string &from, const std::string &to);
	bool     exists(const std::string &path);

	std::shared_ptr<OffscreenSurface> operator[](const std::string &);
	std::shared_ptr<OffscreenSurface> skinRes(const std::string &key, bool useDefault = true);

	std::shared_ptr<OffscreenSurface> add(const std::string &path,
					      unsigned int width = 0,
					      unsigned int height = 0);

private:
	using SurfaceHash = std::unordered_map<std::string, std::shared_ptr<OffscreenSurface>>;

	SurfaceHash surfaces;
	std::string skin;

	GMenu2X *gmenu2x;
};

#endif
