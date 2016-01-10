#include <node.h>
#include <v8.h>
#include <memory>
#include "node.h"
#include "node_buffer.h"

#include <vector>
#include <string>
#include <algorithm>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
namespace AbcA = Alembic::AbcCoreAbstract;

#include "UMAbcSoftwareIO.h"
#include "UMStringUtil.h"
#include "UMAbcScene.h"
#include "UMAbcObject.h"
#include "UMAbcMesh.h"
#include "UMAbcPoint.h"

using namespace v8;

class UMAbcIO {
public:
	typedef std::map<umstring, umabc::UMAbcScenePtr> SceneMap;

	static UMAbcIO& instance() {
		static UMAbcIO abcio;
		return abcio;
	}

	umabc::UMAbcScenePtr get_scene(Isolate* isolate, const FunctionCallbackInfo<Value>& args) {
		if (args.Length() < 1) {
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "Wrong number of arguments")));
			return umabc::UMAbcScenePtr();
		}
		if (!args[0]->IsString()) {
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "Wrong arguments")));
			return umabc::UMAbcScenePtr();
		}

		v8::String::Utf8Value utf8path(args[0]->ToString());
		umstring path = umbase::UMStringUtil::utf8_to_utf16(*utf8path);
		return scene_map_[path];
	}

	void load(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);

		if (args.Length() < 1) {
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "Wrong number of arguments")));
			return;
		}
		if (!args[0]->IsString()) {
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "Wrong arguments")));
			return;
		}

		v8::String::Utf8Value utf8path(args[0]->ToString());
		umstring path = umbase::UMStringUtil::utf8_to_utf16(*utf8path);
		if (scene_map_.find(path) != scene_map_.end()) {
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "Already Loaded")));
		}

		umabc::UMAbcSoftwareIO abcio;
		umabc::UMAbcSetting setting;
		umabc::UMAbcScenePtr scene = abcio.load(path, setting);
		if (scene && scene->init()) {
			scene_map_[path] = scene;
		}
	}

	void save(const FunctionCallbackInfo<Value>& args) {
		// TODO
	}

	void get_total_time(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		umabc::UMAbcScenePtr scene = get_scene(isolate, args);

		Local<Object> time = Object::New(isolate);
		Local<Number> min_time = Number::New(isolate, scene->min_time());
		Local<Number> max_time = Number::New(isolate, scene->max_time());

		time->Set(String::NewFromUtf8(isolate, "min"), min_time);
		time->Set(String::NewFromUtf8(isolate, "max"), max_time);
		args.GetReturnValue().Set(time);
	}

	void get_time(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		umabc::UMAbcScenePtr scene = get_scene(isolate, args);
		Local<Number> time = Number::New(isolate, scene->root_object()->current_time_ms());
		args.GetReturnValue().Set(time);
	}

	void set_time(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		umabc::UMAbcScenePtr scene = get_scene(isolate, args);
		double time = args[1]->NumberValue();
		scene->root_object()->set_current_time(static_cast<unsigned long>(time), true);
	}

	void get_mesh_path_list(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		umabc::UMAbcScenePtr scene = get_scene(isolate, args);
		std::vector<std::string> path_list = scene->mesh_path_list();
		const int list_size = static_cast<int>(path_list.size());
		Local<Array> result = Array::New(isolate, list_size);
		for (int i = 0; i < list_size; ++i) {
			result->Set(i, String::NewFromUtf8(isolate, path_list[i].c_str()));
		}
		args.GetReturnValue().Set(result);
	}

	void get_point_path_list(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		umabc::UMAbcScenePtr scene = get_scene(isolate, args);
		std::vector<std::string> path_list = scene->point_path_list();
		const int list_size = static_cast<int>(path_list.size());
		Local<Array> result = Array::New(isolate, list_size);
		for (int i = 0; i < list_size; ++i) {
			result->Set(i, String::NewFromUtf8(isolate, path_list[i].c_str()));
		}
		args.GetReturnValue().Set(result);
	}

	void get_mesh(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		umabc::UMAbcScenePtr scene = get_scene(isolate, args);

		v8::String::Utf8Value utf8path(args[1]->ToString());
		std::string object_path(*utf8path);
		Local<Object> result = Object::New(isolate);

		umabc::UMAbcMeshPtr mesh = std::dynamic_pointer_cast<umabc::UMAbcMesh>(scene->find_object(object_path));
		if (mesh) {
			if (mesh->vertex_size() > 0)
			{
				Local<ArrayBuffer> vertices = v8::ArrayBuffer::New(isolate, mesh->vertex_size() * sizeof(Imath::V3f));
				ArrayBuffer::Contents contents = vertices->GetContents();
				memcpy(contents.Data(), mesh->vertex(), mesh->vertex_size() * sizeof(Imath::V3f));
				result->Set(String::NewFromUtf8(isolate, "vertex"), Float32Array::New(vertices, 0, mesh->vertex_size() * 3));
			}

			if (mesh->normals().size() > 0)
			{
				Local<ArrayBuffer> normals = v8::ArrayBuffer::New(isolate, mesh->normals().size() * sizeof(Imath::V3f));
				ArrayBuffer::Contents contents = normals->GetContents();
				memcpy(contents.Data(), &mesh->normals()[0], mesh->normals().size() * sizeof(Imath::V3f));
				result->Set(String::NewFromUtf8(isolate, "normal"), Float32Array::New(normals, 0, mesh->normals().size() * 3));
			}

			if (mesh->triangle_index().size() > 0)
			{
				Local<ArrayBuffer> indices = v8::ArrayBuffer::New(isolate, mesh->triangle_index().size() * sizeof(umbase::UMVec3i));
				ArrayBuffer::Contents contents = indices->GetContents();
				memcpy(contents.Data(), &mesh->triangle_index()[0], mesh->triangle_index().size() * sizeof(umbase::UMVec3i));
				result->Set(String::NewFromUtf8(isolate, "index"), Int32Array::New(indices, 0, mesh->triangle_index().size() * 3));
			}

			if (mesh->uv_size() > 0) 
			{
				Local<ArrayBuffer> uvs = v8::ArrayBuffer::New(isolate, mesh->uv_size() * sizeof(Imath::V2f));
				ArrayBuffer::Contents contents = uvs->GetContents();
				memcpy(contents.Data(), mesh->uv(), mesh->uv_size() * sizeof(Imath::V2f));
				result->Set(String::NewFromUtf8(isolate, "uv"), Float32Array::New(uvs, 0, mesh->uv_size() * 2));
			}

			{
				const UMMat44d& global_transform = mesh->global_transform();
				Local<Array> trans = Array::New(isolate, 16);
				for (int i = 0; i < 4; ++i) {
					for (int k = 0; k < 4; ++k) {
						trans->Set(i * 4 + k, Number::New(isolate, global_transform[i][k]));
					}
				}
				result->Set(String::NewFromUtf8(isolate, "global_transform"), trans);
			}

			{
				const UMMat44d& local_transform = mesh->local_transform();
				Local<Array> trans = Array::New(isolate, 16);
				for (int i = 0; i < 4; ++i) {
					for (int k = 0; k < 4; ++k) {
						trans->Set(i * 4 + k, Number::New(isolate, local_transform[i][k]));
					}
				}
				result->Set(String::NewFromUtf8(isolate, "local_transform"), trans);
			}
		}
		args.GetReturnValue().Set(result);
	}

	void get_point(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		umabc::UMAbcScenePtr scene = get_scene(isolate, args);

		v8::String::Utf8Value utf8path(args[1]->ToString());
		std::string object_path(*utf8path);
		Local<Object> result = Object::New(isolate);

		umabc::UMAbcPointPtr point = std::dynamic_pointer_cast<umabc::UMAbcPoint>(scene->find_object(object_path));
		if (point)
		{
			if (point->position_size() > 0)
			{
				Local<ArrayBuffer> positions = v8::ArrayBuffer::New(isolate, point->position_size() * sizeof(Imath::V3f));
				ArrayBuffer::Contents contents = positions->GetContents();
				memcpy(contents.Data(), point->positions(), point->position_size() * sizeof(Imath::V3f));
				result->Set(String::NewFromUtf8(isolate, "position"), Float32Array::New(positions, 0, point->position_size() * 3));
			}

			if (point->normal_size() > 0)
			{
				Local<ArrayBuffer> normals = v8::ArrayBuffer::New(isolate, point->normal_size() * sizeof(Imath::V3f));
				ArrayBuffer::Contents contents = normals->GetContents();
				memcpy(contents.Data(), &point->normals()[0], point->normal_size() * sizeof(Imath::V3f));
				result->Set(String::NewFromUtf8(isolate, "normal"), Float32Array::New(normals, 0, point->normal_size() * 3));
			}

			if (point->color_size() > 0)
			{
				Local<ArrayBuffer> normals = v8::ArrayBuffer::New(isolate, point->color_size() * sizeof(Imath::V3f));
				ArrayBuffer::Contents contents = normals->GetContents();
				memcpy(contents.Data(), &point->colors()[0], point->color_size() * sizeof(Imath::V3f));
				result->Set(String::NewFromUtf8(isolate, "color"), Float32Array::New(normals, 0, point->color_size() * 3));
			}

			{
				const UMMat44d& global_transform = point->global_transform();
				Local<Array> trans = Array::New(isolate, 16);
				for (int i = 0; i < 4; ++i) {
					for (int k = 0; k < 4; ++k) {
						trans->Set(i * 4 + k, Number::New(isolate, global_transform[i][k]));
					}
				}
				result->Set(String::NewFromUtf8(isolate, "global_transform"), trans);
			}

			{
				const UMMat44d& local_transform = point->local_transform();
				Local<Array> trans = Array::New(isolate, 16);
				for (int i = 0; i < 4; ++i) {
					for (int k = 0; k < 4; ++k) {
						trans->Set(i * 4 + k, Number::New(isolate, local_transform[i][k]));
					}
				}
				result->Set(String::NewFromUtf8(isolate, "local_transform"), trans);
			}
		}
		args.GetReturnValue().Set(result);
	}

	void dispose() {
		SceneMap::iterator it = scene_map_.begin();
		for (; it != scene_map_.end(); ++it) {
			it->second = umabc::UMAbcScenePtr();
		}
	}

private:
	SceneMap scene_map_;
};

using node::AtExit;

static void load(const FunctionCallbackInfo<Value>& args)
{
	UMAbcIO::instance().load(args);
}

static void save(const FunctionCallbackInfo<Value>& args)
{
	UMAbcIO::instance().save(args);
}

static void get_total_time(const FunctionCallbackInfo<Value>& args)
{
	UMAbcIO::instance().get_total_time(args);
}

static void get_time(const FunctionCallbackInfo<Value>& args)
{
	UMAbcIO::instance().get_time(args);
}

static void set_time(const FunctionCallbackInfo<Value>& args)
{
	UMAbcIO::instance().set_time(args);
}

static void get_mesh_path_list(const FunctionCallbackInfo<Value>& args)
{
	UMAbcIO::instance().get_mesh_path_list(args);
}

static void get_point_path_list(const FunctionCallbackInfo<Value>& args)
{
	UMAbcIO::instance().get_point_path_list(args);
}

static void get_mesh(const FunctionCallbackInfo<Value>& args)
{
	UMAbcIO::instance().get_mesh(args);
}

static void get_point(const FunctionCallbackInfo<Value>& args)
{
	UMAbcIO::instance().get_point(args);
}

static void dispose(void*)
{
	UMAbcIO::instance().dispose();
}

void Init(Handle<Object> exports) {
	AtExit(dispose);
	NODE_SET_METHOD(exports, "load", load);
	//NODE_SET_METHOD(exports, "save", save);
	NODE_SET_METHOD(exports, "get_total_time", get_total_time);
	NODE_SET_METHOD(exports, "get_time", get_time);
	NODE_SET_METHOD(exports, "set_time", set_time);
	NODE_SET_METHOD(exports, "get_mesh_path_list", get_mesh_path_list);
	NODE_SET_METHOD(exports, "get_point_path_list", get_point_path_list);
	NODE_SET_METHOD(exports, "get_mesh", get_mesh);
	NODE_SET_METHOD(exports, "get_point", get_point);
}

NODE_MODULE(umnode, Init)
