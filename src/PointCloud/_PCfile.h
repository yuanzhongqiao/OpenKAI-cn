/*
 * _PCfile.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCfile_H_
#define OpenKAI_src_PointCloud_PCfile_H_

#include "../Base/common.h"

#ifdef USE_OPEN3D
#include "_PCbase.h"

namespace kai
{

class _PCfile: public _PCbase
{
public:
	_PCfile();
	virtual ~_PCfile();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void draw(void);

private:
	void update(void);
	static void* getUpdate(void* This)
	{
		((_PCfile *) This)->update();
		return NULL;
	}

public:
	string m_fName;

};

}
#endif
#endif
