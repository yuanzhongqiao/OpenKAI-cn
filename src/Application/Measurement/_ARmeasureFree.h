/*
 * _ARmeasureFree.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_Measurement__ARmeasureFree_H_
#define OpenKAI_src_Application_Measurement__ARmeasureFree_H_

#include "_ARmeasure.h"

namespace kai
{
	class _ARmeasureFree : public _StateBase
	{
	public:
		_ARmeasureFree(void);
		virtual ~_ARmeasureFree();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void draw(void *pFrame);

		// callbacks
		static void sOnBtnAction(void *pInst, uint32_t f);
		static void sOnBtnClear(void *pInst, uint32_t f);
		static void sOnBtnMode(void *pInst, uint32_t f);

		void action(void);
		void clear(void);
		void mode(uint32_t f);

	protected:
		void updateFree(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ARmeasureFree *)This)->update();
			return NULL;
		}

		float calcArea(void);

		void drawVertices(Mat* pM);
		void drawMeasure(Mat* pM);

	private:
		_ARmeasure* m_pA;
		_Remap *m_pR;
		_WindowCV *m_pW;

		bool m_bStarted;

		// result
		float m_area;
		float m_Dtot;

		// vertices
		vector<ARMEASURE_VERTEX> m_vVert;

		// draw
		Scalar m_drawCol;
		cv::Ptr<freetype::FreeType2> m_pFt;
	};

}
#endif
