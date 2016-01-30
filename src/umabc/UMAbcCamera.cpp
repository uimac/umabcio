/**
 * @file UMAbcCamera.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMAbcCamera.h"

//#include "UMCamera.h"

#include <algorithm>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreFactory/All.h>

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;

	class UMAbcCamera::Impl : public UMAbcObject
	{
		DISALLOW_COPY_AND_ASSIGN(Impl);
	public:
		Impl(ICameraPtr camera)
			: UMAbcObject(camera)
			, camera_(camera)
		{
			//umcamera_ = std::make_shared<umdraw::UMCamera>(false, 800, 600);
		}
		~Impl() {}

		bool init(bool recursive);

		void set_current_time(unsigned long time, bool recursive);

		///**
		//* get umcamera
		//*/
		//umdraw::UMCameraPtr umcamera() const
		//{
		//	return umcamera_;
		//}

		virtual UMAbcObjectPtr self_reference()
		{
			return self_reference_.lock();
		}
		
		UMAbcCameraWeakPtr self_reference_;

	private:
		ICameraPtr camera_;
		Alembic::AbcGeom::CameraSample sample_;

		//umdraw::UMCameraPtr umcamera_;
	};


bool UMAbcCamera::Impl::init(bool recursive)
{
	if (!is_valid()) return false;

	ICameraSchema &schema = camera_->getSchema();

	const size_t num_samples = schema.getNumSamples();
	if (num_samples > 0)
	{
		// get sample
		schema.get(sample_);

		// if not consistant, we get time
		if (!schema.isConstant())
		{
			TimeSamplingPtr time = schema.getTimeSampling();
		}
	}
	return true;
}

void UMAbcCamera::Impl::set_current_time(unsigned long time, bool recursive)
{
	if (!is_valid()) {
		mutable_local_transform().identity();
		return;
	}

	//if (umcamera_)
	//{
	//	UMAbcNodePtr p = self_reference()->parent();
	//	UMMat44d mat = p->local_transform();
	//	UMMat44d gmat = p->global_transform();

	//	umcamera_->mutable_global_transform() = mat;
	//	umcamera_->update_from_node();
	//}
}

/**
 * create
 */
UMAbcCameraPtr UMAbcCamera::create(ICameraPtr camera)
{
	UMAbcCameraPtr instance = UMAbcCameraPtr(new UMAbcCamera(camera));
	instance->impl_->self_reference_ = instance;
	return instance;
}

/**
 * construtor
 */
UMAbcCamera::UMAbcCamera(ICameraPtr camera)
	: UMAbcObject(camera)
	, impl_(new UMAbcCamera::Impl(camera))
{
}

/**
 * destructor
 */
UMAbcCamera::~UMAbcCamera()
{
}

/**
 * initialize
 */
bool UMAbcCamera::init(bool recursive, UMAbcObjectPtr parent)
{
	impl_->init(recursive);
	return UMAbcObject::init(recursive, parent);
}

/**
 * set current time
 */
void UMAbcCamera::set_current_time(unsigned long time, bool recursive)
{
	impl_->set_current_time(time, recursive);
	UMAbcObject::set_current_time(time, recursive);
}

/**
 * update box
 */
void UMAbcCamera::update_box(bool recursive)
{
}

///**
//* get umcamera
//*/
//umdraw::UMCameraPtr UMAbcCamera::umcamera() const
//{
//	return impl_->umcamera();
//}

UMAbcObjectPtr UMAbcCamera::self_reference()
{
	return impl_->self_reference();
}

}

