/**
 * @file UMAbcNurbsPatch.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include <algorithm>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreFactory/All.h>

#include "UMAbcNurbsPatch.h"

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;
	
	class UMAbcNurbsPatch::Impl : public UMAbcObject
	{
		DISALLOW_COPY_AND_ASSIGN(Impl);
	public:
		Impl(INuPatchPtr patch)
			: UMAbcObject(patch)
			, patch_(patch)
			, u_size_(0)
			, v_size_(0)
			, u_order_(0)
			, v_order_(0)
		{}

		~Impl() {}


		/**
		* initialize
		* @param [in] recursive do children recursively
		* @retval succsess or fail
		*/
		virtual bool init(bool recursive);

		/**
		* set current time
		* @param [in] time time
		* @param [in] recursive do children recursively
		*/
		virtual void set_current_time(unsigned long time, bool recursive);

		/**
		* update box
		* @param [in] recursive do children recursively
		*/
		virtual void update_box(bool recursive);

		/**
		* update patch all
		*/
		void update_patch_all();

		virtual UMAbcObjectPtr self_reference()
		{
			return self_reference_.lock();
		}

		UMAbcNurbsPatchWeakPtr self_reference_;

		Alembic::AbcGeom::P3fArraySamplePtr positions() const { return positions_; }

		Alembic::AbcGeom::FloatArraySamplePtr u_knot_list() const { return v_knot_; }
		Alembic::AbcGeom::FloatArraySamplePtr v_knot_list() const { return u_knot_; }

		unsigned int u_size() const { return u_size_; }
		unsigned int v_size() const { return v_size_; }

		int u_order() const { return u_order_; }
		int v_order() const { return v_order_; }

	private:
		INuPatchPtr patch_;
		Alembic::AbcGeom::INuPatchSchema::Sample initial_sample_;

		Alembic::AbcGeom::P3fArraySamplePtr positions_;
		Alembic::AbcGeom::FloatArraySamplePtr u_knot_;
		Alembic::AbcGeom::FloatArraySamplePtr v_knot_;
		size_t u_size_;
		size_t v_size_;
		int u_order_;
		int v_order_;

		std::vector<const Imath::V3f* > points_;
	};
/**
 * create
 */
UMAbcNurbsPatchPtr UMAbcNurbsPatch::create(INuPatchPtr patch)
{
	UMAbcNurbsPatchPtr instance = UMAbcNurbsPatchPtr(new UMAbcNurbsPatch(patch));
	instance->impl_->self_reference_ = instance;
	return instance;
}

UMAbcNurbsPatch::UMAbcNurbsPatch(INuPatchPtr patch)
	: UMAbcObject(patch)
	, impl_(new UMAbcNurbsPatch::Impl(patch))
{}


UMAbcNurbsPatch::~UMAbcNurbsPatch()
{
}

/**
 * init
 */
bool UMAbcNurbsPatch::Impl::init(bool recursive)
{
	if (!is_valid()) return false;
	
	size_t num_samples = patch_->getSchema().getNumSamples();
	if (num_samples > 0)
	{
		// get constant sample
		patch_->getSchema().get(initial_sample_);
		
		// if not consistant, we get time
		if (!patch_->getSchema().isConstant())
		{
			TimeSamplingPtr time = patch_->getSchema().getTimeSampling();
			self_reference()->set_min_time(static_cast<unsigned long>(time->getSampleTime(0) * 1000));
			self_reference()->set_max_time(static_cast<unsigned long>(time->getSampleTime(num_samples - 1) * 1000));
		}
	}

	return false;
}

/**
 * set_current_time
 */
void UMAbcNurbsPatch::Impl::set_current_time(unsigned long time, bool recursive)
{
	if (!is_valid()) return;
	update_patch_all();
}

/**
 * update patch all
 */
void UMAbcNurbsPatch::Impl::update_patch_all()
{
	if (!is_valid()) return;
	ISampleSelector selector(self_reference()->current_time(), ISampleSelector::kNearIndex);
	INuPatchSchema::Sample sample;
	patch_->getSchema().get(sample, selector);

	positions_ = sample.getPositions();
	u_knot_ = sample.getUKnot();
	v_knot_ = sample.getVKnot();
	u_size_ = sample.getNumU();
	v_size_ = sample.getNumV();
	u_order_ = sample.getUOrder();
	v_order_ = sample.getVOrder();
}

/**
 * update box
 */
void UMAbcNurbsPatch::Impl::update_box(bool recursive)
{
	if (!is_valid()) return;

	mutable_box().makeEmpty();
	if (positions_)
	{
		size_t position_count = positions_->size();
		for (size_t i = 0; i < position_count; ++i)
		{
			const Imath::V3f& p = (*positions_)[i];
			mutable_box().extendBy(p);
		}
	}
}

/**
* initialize
* @param [in] recursive do children recursively
* @retval succsess or fail
*/
bool UMAbcNurbsPatch::init(bool recursive, UMAbcObjectPtr parent)
{
	impl_->init(recursive);
	return UMAbcObject::init(recursive, parent);
}

/**
* set current time
* @param [in] time time
* @param [in] recursive do children recursively
*/
void UMAbcNurbsPatch::set_current_time(unsigned long time, bool recursive)
{
	if (!impl_->is_valid()) return;
	UMAbcObject::set_current_time(time, recursive);
	impl_->set_current_time(time, recursive);
}

/**
* get position
*/
const Imath::V3f * UMAbcNurbsPatch::positions() const
{
	if (impl_->positions()) {
		return impl_->positions()->get();
	}
	return NULL;
}

/**
* get position size
*/
unsigned int UMAbcNurbsPatch::position_size() const
{
	if (impl_->positions()) {
		return impl_->positions()->size();
	}
	return 0;
}

/**
* get u knots
*/
const float * UMAbcNurbsPatch::u_knots() const
{
	if (impl_->u_knot_list()) {
		return impl_->u_knot_list()->get();
	}
	return NULL;
}

/**
* get u knots size
*/
unsigned int UMAbcNurbsPatch::u_knot_size() const
{
	if (impl_->u_knot_list()) {
		return impl_->u_knot_list()->size();
	}
	return 0;
}

/**
* get v knots
*/
const float * UMAbcNurbsPatch::v_knots() const
{
	if (impl_->v_knot_list()) {
		return impl_->v_knot_list()->get();
	}
	return 0;
}

/**
* get v knots size
*/
unsigned int UMAbcNurbsPatch::v_knot_size() const
{
	if (impl_->v_knot_list()) {
		return impl_->v_knot_list()->size();
	}
	return 0;
}

/**
* get u_size
*/
unsigned int UMAbcNurbsPatch::u_size() const
{
	return impl_->u_size();
}

/**
* get v_size
*/
unsigned int UMAbcNurbsPatch::v_size() const
{
	return impl_->v_size();
}

/**
* get u_order
*/
int UMAbcNurbsPatch::u_order() const
{
	return impl_->u_order();
}

/**
* get v_order
*/
int UMAbcNurbsPatch::v_order() const
{
	return impl_->v_order();
}

/**
* update box
* @param [in] recursive do children recursively
*/
void UMAbcNurbsPatch::update_box(bool recursive)
{
	return impl_->update_box(recursive);
}

/**
* update patch all
*/
void UMAbcNurbsPatch::update_patch_all()
{
	impl_->update_patch_all();
}

UMAbcObjectPtr UMAbcNurbsPatch::self_reference()
{
	return impl_->self_reference_.lock();
}

}

