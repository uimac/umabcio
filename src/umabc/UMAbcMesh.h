/**
 * @file UMAbcMesh.h
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
#include "ImathVec.h"
#include "UMMacro.h"
#include "UMAbcObject.h"

namespace Alembic
{
	namespace Abc {
		namespace v7 {
			template <class SCHEMA>
			class ISchemaObject;
		}
	}
	namespace AbcGeom {
		namespace v7 {
			class IPolyMeshSchema;
			typedef Alembic::Abc::v7::ISchemaObject<IPolyMeshSchema> IPolyMesh;
		}
	}
}

namespace umabc
{
typedef std::shared_ptr<Alembic::AbcGeom::v7::IPolyMesh> IPolyMeshPtr;

class UMAbcMesh;
typedef std::shared_ptr<UMAbcMesh> UMAbcMeshPtr;
typedef std::weak_ptr<UMAbcMesh> UMAbcMeshWeakPtr;

class UMAbcMesh : public UMAbcObject
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcMesh);
public:
	typedef std::vector<Imath::V3i > IndexList;

	/**
	 * crate instance
	 */
	static UMAbcMeshPtr create(IPolyMeshPtr poly_mesh);

	~UMAbcMesh();

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

	///**
	// * draw
	// * @param [in] recursive do children recursively
	// */
	//virtual void draw(bool recursive, UMAbc::DrawType type);

	/**
	* get current time
	*/
	virtual double current_time() const;

	/**
	 * get polygon count
	 */
	int polygon_count() const;
	
	/** 
	 * update mesh all
	 */
	void update_mesh_all();

	/**
	 * get faceset name list
	 */
	const std::vector<std::string>& faceset_name_list() const;

	/**
	 * get faceset polycount list
	 */
	const std::vector<int>& faceset_polycount_list() const;

	/**
	 * get vertex
	 */
	const Imath::V3f * vertex() const;

	/**
	* get vertex size
	*/
	unsigned int vertex_size() const;

	/**
	 * get uv
	 */
	const Imath::V2f * uv() const;

	/**
	 * get uv size
	 */
	unsigned int uv_size() const;

	/**
	 * get triangle index list
	 */
	IndexList& triangle_index();

	/**
	 * get normal
	 */
	std::vector<Imath::V3f>& normals();

protected:
	UMAbcMesh(IPolyMeshPtr poly_mesh);
	
	virtual UMAbcObjectPtr self_reference();

private:
	class Impl;
	std::unique_ptr <Impl> impl_;
};

} // umabc
