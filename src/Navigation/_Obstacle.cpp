/*
 * _Obstacle.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#include "_Obstacle.h"

namespace kai
{

_Obstacle::_Obstacle()
{
	m_pZed = NULL;
	m_pMatrix = NULL;
	m_nFilter = 0;
	m_dBlend = 0.5;

	m_mDim.x = 10;
	m_mDim.y = 10;

	m_range.x = 0.0;
	m_range.y = DBL_MAX;
}

_Obstacle::~_Obstacle()
{
	DEL(m_pMatrix);

	for (int i = 0; i < m_nFilter; i++)
		DEL(m_pFilteredMatrix[i]);
}

bool _Obstacle::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("dBlend", &m_dBlend));
	F_INFO(pK->v("matrixW", &m_mDim.x));
	F_INFO(pK->v("matrixH", &m_mDim.y));

	m_nFilter = m_mDim.area();
	IF_F(m_nFilter >= N_FILTER);

	Kiss* pCC;
	int i;

	//filter
	pCC = pK->o("medianFilter");
	IF_F(pCC->empty());

	for (i = 0; i < m_nFilter; i++)
	{
		m_pFilteredMatrix[i] = new Median();
		IF_F(!m_pFilteredMatrix[i]->init(pCC));
	}

	m_pMatrix = new Frame();

	return true;
}

bool _Obstacle::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_ZED", &iName));
	m_pZed = (_ZED*) (pK->root()->getChildInstByName(&iName));

	IF_F(!m_pZed);
	m_range = m_pZed->getRange();

	return true;
}

bool _Obstacle::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Obstacle::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _Obstacle::detect(void)
{
	NULL_(m_pZed);
	IF_(!m_pZed->isOpened());

	Frame* pDepth = m_pZed->depth();
	NULL_(pDepth);
	IF_(pDepth->empty());

	m_pMatrix->getResizedOf(m_pZed->depth(), m_mDim.x, m_mDim.y);
	Mat* pM = m_pMatrix->getCMat();

	int i,j;
	for(i=0;i<m_mDim.y;i++)
	{
		for(j=0;j<m_mDim.x;j++)
		{
			m_pFilteredMatrix[i*m_mDim.x+j]->input((double)pM->at<float>(i,j));
		}
	}
}

double _Obstacle::dist(vDouble4* pROI, vInt2* pPos)
{
	if(!m_pZed)return -1.0;
	if(!pROI)return -1.0;

	vInt4 roi;
	roi.x = pROI->x * m_mDim.x;
	roi.y = pROI->y * m_mDim.y;
	roi.z = pROI->z * m_mDim.x;
	roi.w = pROI->w * m_mDim.y;
	if(roi.x<0)roi.x=0;
	if(roi.y<0)roi.y=0;
	if(roi.z>=m_mDim.x)roi.z=m_mDim.x-1;
	if(roi.w>=m_mDim.y)roi.w=m_mDim.y-1;

	double dMin = m_range.y;
	vInt2 posMin;
	int i,j;

	for(i=roi.y;i<roi.w;i++)
	{
		for(j=roi.x;j<roi.z;j++)
		{
			double dCell = m_pFilteredMatrix[i*m_mDim.x+j]->v();
			IF_CONT(dCell < m_range.x);
			IF_CONT(dCell > m_range.y);
			IF_CONT(dCell > dMin);

			dMin = dCell;
			posMin.x = j;
			posMin.y = i;
		}
	}

	if(pPos)
	{
		*pPos = posMin;
	}

	return dMin;
}

vInt2 _Obstacle::matrixDim(void)
{
	return m_mDim;
}

vDouble2 _Obstacle::getRange(void)
{
	return m_range;
}

DISTANCE_SENSOR_TYPE _Obstacle::getType(void)
{
	return dsZED;
}

bool _Obstacle::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	IF_F(pMat->empty());
	NULL_F(m_pZed);
	IF_F(m_pMatrix->empty());

	Mat mM = *m_pMatrix->getCMat();
	IF_F(mM.empty());

	double normD;
	double baseD = 255.0/(m_range.y - m_range.x);

    Mat filterM = Mat::zeros(Size(m_mDim.x,m_mDim.y), CV_8UC1);
	int i,j;
	for(i=0;i<m_mDim.y;i++)
	{
		for(j=0;j<m_mDim.x;j++)
		{
			normD = m_pFilteredMatrix[i*m_mDim.x+j]->v();
			normD = (normD - m_range.x) * baseD;
			filterM.at<uchar>(i,j) = 255 - (uchar)normD;
		}
	}

	Mat mA;
    mA = Mat::zeros(Size(m_mDim.x,m_mDim.y), CV_8UC1);
	vector<Mat> channels;
    channels.push_back(mA);
    channels.push_back(mA);
    channels.push_back(filterM);
    Mat mB;
    cv::merge(channels, mB);

	cv::resize(mB, mA, Size(pMat->cols, pMat->rows),0,0,INTER_NEAREST);
	cv::addWeighted(*pMat, 1.0, mA, m_dBlend, 0.0, *pMat);

	return true;
}

}
