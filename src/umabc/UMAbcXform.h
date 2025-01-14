/**
 * @file UMAbcXform.h
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

#include "UMMacro.h"
#include "UMAbcObject.h"

namespace Alembic {
	namespace Abc {
		namespace v7 {
			template <class SCHEMA>
			class ISchemaObject;
		}
	}
	namespace AbcGeom {
		namespace v7 {
			class IXformSchema;
			typedef Alembic::Abc::v7::ISchemaObject<IXformSchema> IXform;
		}
	}
}

namespace umabc
{
	typedef std::shared_ptr<Alembic::AbcGeom::v7::IXform> IXformPtr;

class UMAbcXform;
typedef std::shared_ptr<UMAbcXform> UMAbcXformPtr;
typedef std::weak_ptr<UMAbcXform> UMAbcXformWeakPtr;

class UMAbcXform : public UMAbcObject
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcXform);
public:
	static UMAbcXformPtr create(IXformPtr xform);

	~UMAbcXform();

	/**
	 * initialize
	 * @param [in] recursive do children recursively
	 * @retval succsess or fail
	 */
	virtual bool init(bool recursive, UMAbcObjectPtr parent);
	
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
	* get current time
	*/
	virtual double current_time() const;

protected:
	UMAbcXform(IXformPtr xform);
	
	virtual UMAbcObjectPtr self_reference();

private:
	class Impl;
	std::unique_ptr<Impl> impl_;
};

} // umabc
