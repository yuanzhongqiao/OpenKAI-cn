/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "../JetsonInference/_DetectNet.h"

#ifdef USE_OPENCV
#ifdef USE_JETSON_INFERENCE

namespace kai
{

_DetectNet::_DetectNet()
{
	m_pRGBA = NULL;
	m_pRGBAf = NULL;

	m_pDN = NULL;
	m_nBox = 0;
	m_nClass = 0;
	m_type = detectNet_uff;
	m_thr = 0.5;
	m_layerIn = "Input";
	m_layerOut = "NMS";
	m_layerNboxOut = "NMS_1";
	m_vDimIn.init(3,300,300);
	m_nMaxBatch = DEFAULT_MAX_BATCH_SIZE;
	m_precision = TYPE_FASTEST;
	m_device = DEVICE_GPU;
	m_bAllowGPUfallback = true;

	m_bSwapRB = false;
	m_vMean.init(0.0);

}

_DetectNet::~_DetectNet()
{
	DEL(m_pRGBA);
	DEL(m_pRGBAf);
}

bool _DetectNet::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("thr", &m_thr);
	pK->v("bSwapRB", &m_bSwapRB);
	pK->v("vMean", &m_vMean);
	pK->v("type", &m_type);

	m_pRGBA = new Frame();
	m_pRGBAf = new Frame();

	return true;
}

bool _DetectNet::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _DetectNet::check(void)
{
	NULL__(m_pV, -1);
	NULL__(m_pU,-1);
	NULL__(m_pDN, -1);
	Frame* pBGR = m_pV->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);

	return 0;
}

bool _DetectNet::open(void)
{
	if(m_type==detectNet_uff)
	{
		m_pDN = Create(m_fModel,
						m_fClass,
						m_thr,
						m_layerIn.c_str(),
						Dims3(m_vDimIn.x, m_vDimIn.y, m_vDimIn.z),
						m_layerOut.c_str(),
						m_layerNboxOut.c_str(),
						m_nMaxBatch,
						m_precision,
						m_device,
						m_bAllowGPUfallback);
	}

	NULL_F(m_pDN);

	m_nClass = m_pDN->GetNumClasses();
	m_pDN->SetThreshold(m_thr);

	return true;
}

void _DetectNet::update(void)
{
	open();

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();
		m_pU->updateObj();

		this->autoFPSto();
	}

}

void _DetectNet::detect(void)
{
	IF_(check()<0);

	*m_pRGBA = m_pV->BGR()->cvtColor(CV_BGR2RGBA);
	*m_pRGBAf = m_pRGBA->cvtTo(CV_32FC4);
	GpuMat fGMat = *m_pRGBAf->gm();

	detectNet::Detection* pBox = NULL;
	m_nBox = m_pDN->Detect((float*)fGMat.data, fGMat.cols, fGMat.rows, &pBox, OVERLAY_NONE);
	IF_(m_nBox <= 0);

	float kx = 1.0/(float)fGMat.cols;
	float ky = 1.0/(float)fGMat.rows;

	for( int i=0; i < m_nBox; i++ )
	{
		detectNet::Detection* pB = &pBox[i];

		_Object o;
		o.init();
		o.m_tStamp = m_tStamp;
		o.setTopClass(pB->ClassID, pB->Confidence);
		o.setText(m_pDN->GetClassDesc(pB->ClassID));
		o.setBB2D(pB->Left, pB->Top, pB->Width(), pB->Height());
		o.scale(kx,ky);

		add(&o);
		LOG_I("BBox: "<< o.getText() << " Prob: " << f2str(o.getTopClassProb()));
	}

}

bool _DetectNet::draw(void)
{
	IF_F(!this->_DetectorBase::draw());

	return true;
}

}
#endif
#endif

