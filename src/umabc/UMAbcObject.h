/**
 * @file UMAbcObject.h
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <string>
#include "ImathBox.h"

#include "UMMacro.h"
#include "UMAbcNode.h"

namespace Alembic {
	namespace Abc {
		namespace v7 {
			class IObject;
		}
	}
}

namespace umabc
{
typedef std::shared_ptr<Alembic::Abc::v7::IObject> IObjectPtr;

class UMAbcObject;
typedef std::shared_ptr<UMAbcObject> UMAbcObjectPtr;
typedef std::weak_ptr<UMAbcObject> UMAbcObjectWeakPtr;
typedef std::vector<UMAbcObjectPtr> UMAbcObjectList;
class UMAbcObject : public UMAbcNode
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcObject);
public:
	/**
	 * crate instance
	 */
	static UMAbcObjectPtr create(IObjectPtr object);

	virtual ~UMAbcObject();
	
	/**
	 * initialize
	 * @param [in] recursive do children recursively
	 */
	virtual bool init(bool recursive, UMAbcObjectPtr parent);

	/**
	 * get minumum time
	 */
	virtual unsigned long min_time() const;

	/**
	 * get maximum time
	 */
	virtual unsigned long max_time() const;

	/**
	 * set current time
	 */
	virtual void set_current_time(unsigned long time, bool recursive);

	/**
	 * get current time
	 */
	virtual double current_time() const;

	/**
	 * get current time
	 */
	unsigned int current_time_ms() const;

	/**
	 * is valid or not
	 */
	virtual bool is_valid() const;

	/**
	 * is visible or not
	 */
	virtual bool is_visible() const;

	/**
	 * get bounding box
	 */
	virtual const Imath::Box3d& box() const;

	/**
	 * get no inherit bounding box
	 */
	virtual const Imath::Box3d& no_inherit_box() const;

	/**
	 * update box
	 * @param [in] recursive do children recursively
	 */
	virtual void update_box(bool recursive);

	///**
	// * draw
	// */
	//virtual void draw(bool recursive, UMAbc::DrawType type);

	/**
	 * get children
	 */
	const UMAbcObjectList& children() const;

	/**
	 * get children
	 */
	UMAbcObjectList& mutable_children();

	/**
	 * get name
	 */
	const std::string& name() const;


	/**
	* get minumum time
	*/
	virtual void set_min_time(unsigned long time);

	/**
	* get maximum time
	*/
	virtual void set_max_time(unsigned long time);

	/**
	 * get raw object
	 */
	IObjectPtr object();

protected:
	virtual UMAbcObjectPtr self_reference();

	UMAbcObject(IObjectPtr object);

	/**
	* get bounding box
	*/
	virtual Imath::Box3d& mutable_box();

	/**
	* get no inherit bounding box
	*/
	virtual Imath::Box3d& mutable_no_inherit_box();

private:
	class Impl;
	std::unique_ptr<Impl> impl_;
};

} // umabc
