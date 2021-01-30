/*************************************************************************/
/*  register_core_types.cpp                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "register_core_types.h"

#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/core_bind.h"
#include "core/core_string_names.h"
#include "core/crypto/aes_context.h"
#include "core/crypto/crypto.h"
#include "core/crypto/hashing_context.h"
#include "core/input/input.h"
#include "core/input/input_map.h"
#include "core/io/config_file.h"
#include "core/io/dtls_server.h"
#include "core/io/http_client.h"
#include "core/io/image_loader.h"
#include "core/io/json.h"
#include "core/io/marshalls.h"
#include "core/io/multiplayer_api.h"
#include "core/io/networked_multiplayer_peer.h"
#include "core/io/packed_data_container.h"
#include "core/io/packet_peer.h"
#include "core/io/packet_peer_dtls.h"
#include "core/io/packet_peer_udp.h"
#include "core/io/pck_packer.h"
#include "core/io/resource_format_binary.h"
#include "core/io/resource_importer.h"
#include "core/io/stream_peer_ssl.h"
#include "core/io/tcp_server.h"
#include "core/io/translation_loader_po.h"
#include "core/io/udp_server.h"
#include "core/io/xml_parser.h"
#include "core/math/a_star.h"
#include "core/math/expression.h"
#include "core/math/geometry_2d.h"
#include "core/math/geometry_3d.h"
#include "core/math/random_number_generator.h"
#include "core/math/triangle_mesh.h"
#include "core/object/class_db.h"
#include "core/object/undo_redo.h"
#include "core/os/main_loop.h"
#include "core/string/compressed_translation.h"
#include "core/string/translation.h"

static Ref<ResourceFormatSaverBinary> resource_saver_binary;
static Ref<ResourceFormatLoaderBinary> resource_loader_binary;
static Ref<ResourceFormatImporter> resource_format_importer;
static Ref<ResourceFormatLoaderImage> resource_format_image;
static Ref<TranslationLoaderPO> resource_format_po;
static Ref<ResourceFormatSaverCrypto> resource_format_saver_crypto;
static Ref<ResourceFormatLoaderCrypto> resource_format_loader_crypto;

static _ResourceLoader *_resource_loader = nullptr;
static _ResourceSaver *_resource_saver = nullptr;
static _OS *_os = nullptr;
static _Engine *_engine = nullptr;
static _ClassDB *_classdb = nullptr;
static _Marshalls *_marshalls = nullptr;
static _JSON *_json = nullptr;
static _EngineDebugger *_engine_debugger = nullptr;

static IP *ip = nullptr;

static _Geometry2D *_geometry_2d = nullptr;
static _Geometry3D *_geometry_3d = nullptr;

extern Mutex _global_mutex;

extern void register_global_constants();
extern void unregister_global_constants();

void register_core_types() {
	//consistency check
	static_assert(sizeof(Callable) <= 16);

	ObjectDB::setup();

	StringName::setup();
	ResourceLoader::initialize();

	register_global_constants();

	Variant::register_types();

	CoreStringNames::create();

	resource_format_po.instance();
	ResourceLoader::add_resource_format_loader(resource_format_po);

	resource_saver_binary.instance();
	ResourceSaver::add_resource_format_saver(resource_saver_binary);
	resource_loader_binary.instance();
	ResourceLoader::add_resource_format_loader(resource_loader_binary);

	resource_format_importer.instance();
	ResourceLoader::add_resource_format_loader(resource_format_importer);

	resource_format_image.instance();
	ResourceLoader::add_resource_format_loader(resource_format_image);

	ClassDB::register_class<Object>();

	ClassDB::register_virtual_class<Script>();

	ClassDB::register_class<Reference>();
	ClassDB::register_class<WeakRef>();
	ClassDB::register_class<Resource>();
	ClassDB::register_class<Image>();

	ClassDB::register_virtual_class<InputEvent>();
	ClassDB::register_virtual_class<InputEventWithModifiers>();
	ClassDB::register_virtual_class<InputEventFromWindow>();
	ClassDB::register_class<InputEventKey>();
	ClassDB::register_virtual_class<InputEventMouse>();
	ClassDB::register_class<InputEventMouseButton>();
	ClassDB::register_class<InputEventMouseMotion>();
	ClassDB::register_class<InputEventJoypadButton>();
	ClassDB::register_class<InputEventJoypadMotion>();
	ClassDB::register_class<InputEventScreenDrag>();
	ClassDB::register_class<InputEventScreenTouch>();
	ClassDB::register_class<InputEventAction>();
	ClassDB::register_virtual_class<InputEventGesture>();
	ClassDB::register_class<InputEventMagnifyGesture>();
	ClassDB::register_class<InputEventPanGesture>();
	ClassDB::register_class<InputEventMIDI>();

	ClassDB::register_virtual_class<StreamPeer>();
	ClassDB::register_class<StreamPeerBuffer>();
	ClassDB::register_class<StreamPeerTCP>();
	ClassDB::register_class<TCP_Server>();
	ClassDB::register_class<PacketPeerUDP>();
	ClassDB::register_class<UDPServer>();
	ClassDB::register_custom_instance_class<PacketPeerDTLS>();
	ClassDB::register_custom_instance_class<DTLSServer>();

	// Crypto
	ClassDB::register_class<HashingContext>();
	ClassDB::register_class<AESContext>();
	ClassDB::register_custom_instance_class<X509Certificate>();
	ClassDB::register_custom_instance_class<CryptoKey>();
	ClassDB::register_custom_instance_class<HMACContext>();
	ClassDB::register_custom_instance_class<Crypto>();
	ClassDB::register_custom_instance_class<StreamPeerSSL>();

	resource_format_saver_crypto.instance();
	ResourceSaver::add_resource_format_saver(resource_format_saver_crypto);
	resource_format_loader_crypto.instance();
	ResourceLoader::add_resource_format_loader(resource_format_loader_crypto);

	ClassDB::register_virtual_class<IP>();
	ClassDB::register_virtual_class<PacketPeer>();
	ClassDB::register_class<PacketPeerStream>();
	ClassDB::register_virtual_class<NetworkedMultiplayerPeer>();
	ClassDB::register_class<MultiplayerAPI>();
	ClassDB::register_class<MainLoop>();
	ClassDB::register_class<Translation>();
	ClassDB::register_class<PHashTranslation>();
	ClassDB::register_class<UndoRedo>();
	ClassDB::register_class<HTTPClient>();
	ClassDB::register_class<TriangleMesh>();

	ClassDB::register_class<ResourceFormatLoader>();
	ClassDB::register_class<ResourceFormatSaver>();

	ClassDB::register_class<_File>();
	ClassDB::register_class<_Directory>();
	ClassDB::register_class<_Thread>();
	ClassDB::register_class<_Mutex>();
	ClassDB::register_class<_Semaphore>();

	ClassDB::register_class<XMLParser>();
	ClassDB::register_class<JSONParser>();

	ClassDB::register_class<ConfigFile>();

	ClassDB::register_class<PCKPacker>();

	ClassDB::register_class<PackedDataContainer>();
	ClassDB::register_virtual_class<PackedDataContainerRef>();
	ClassDB::register_class<AStar>();
	ClassDB::register_class<AStar2D>();
	ClassDB::register_class<EncodedObjectAsID>();
	ClassDB::register_class<RandomNumberGenerator>();

	ClassDB::register_class<JSONParseResult>();

	ClassDB::register_virtual_class<ResourceImporter>();

	ip = IP::create();

	_geometry_2d = memnew(_Geometry2D);
	_geometry_3d = memnew(_Geometry3D);

	_resource_loader = memnew(_ResourceLoader);
	_resource_saver = memnew(_ResourceSaver);
	_os = memnew(_OS);
	_engine = memnew(_Engine);
	_classdb = memnew(_ClassDB);
	_marshalls = memnew(_Marshalls);
	_json = memnew(_JSON);
	_engine_debugger = memnew(_EngineDebugger);
}

void register_core_settings() {
	// Since in register core types, globals may not be present.
	GLOBAL_DEF("network/limits/tcp/connect_timeout_seconds", (30));
	ProjectSettings::get_singleton()->set_custom_property_info("network/limits/tcp/connect_timeout_seconds", PropertyInfo(Variant::INT, "network/limits/tcp/connect_timeout_seconds", PROPERTY_HINT_RANGE, "1,1800,1"));
	GLOBAL_DEF_RST("network/limits/packet_peer_stream/max_buffer_po2", (16));
	ProjectSettings::get_singleton()->set_custom_property_info("network/limits/packet_peer_stream/max_buffer_po2", PropertyInfo(Variant::INT, "network/limits/packet_peer_stream/max_buffer_po2", PROPERTY_HINT_RANGE, "0,64,1,or_greater"));

	GLOBAL_DEF("network/ssl/certificate_bundle_override", "");
	ProjectSettings::get_singleton()->set_custom_property_info("network/ssl/certificate_bundle_override", PropertyInfo(Variant::STRING, "network/ssl/certificate_bundle_override", PROPERTY_HINT_FILE, "*.crt"));
}

void register_core_singletons() {
	ClassDB::register_class<ProjectSettings>();
	ClassDB::register_virtual_class<IP>();
	ClassDB::register_class<_Geometry2D>();
	ClassDB::register_class<_Geometry3D>();
	ClassDB::register_class<_ResourceLoader>();
	ClassDB::register_class<_ResourceSaver>();
	ClassDB::register_class<_OS>();
	ClassDB::register_class<_Engine>();
	ClassDB::register_class<_ClassDB>();
	ClassDB::register_class<_Marshalls>();
	ClassDB::register_class<TranslationServer>();
	ClassDB::register_virtual_class<Input>();
	ClassDB::register_class<InputMap>();
	ClassDB::register_class<_JSON>();
	ClassDB::register_class<Expression>();
	ClassDB::register_class<_EngineDebugger>();

	Engine::get_singleton()->add_singleton(Engine::Singleton("ProjectSettings", ProjectSettings::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("IP", IP::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Geometry2D", _Geometry2D::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Geometry3D", _Geometry3D::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("ResourceLoader", _ResourceLoader::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("ResourceSaver", _ResourceSaver::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("OS", _OS::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Engine", _Engine::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("ClassDB", _classdb));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Marshalls", _Marshalls::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("TranslationServer", TranslationServer::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("Input", Input::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("InputMap", InputMap::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("JSON", _JSON::get_singleton()));
	Engine::get_singleton()->add_singleton(Engine::Singleton("EngineDebugger", _EngineDebugger::get_singleton()));
}

void unregister_core_types() {
	memdelete(_resource_loader);
	memdelete(_resource_saver);
	memdelete(_os);
	memdelete(_engine);
	memdelete(_classdb);
	memdelete(_marshalls);
	memdelete(_json);
	memdelete(_engine_debugger);

	memdelete(_geometry_2d);
	memdelete(_geometry_3d);

	ResourceLoader::remove_resource_format_loader(resource_format_image);
	resource_format_image.unref();

	ResourceSaver::remove_resource_format_saver(resource_saver_binary);
	resource_saver_binary.unref();

	ResourceLoader::remove_resource_format_loader(resource_loader_binary);
	resource_loader_binary.unref();

	ResourceLoader::remove_resource_format_loader(resource_format_importer);
	resource_format_importer.unref();

	ResourceLoader::remove_resource_format_loader(resource_format_po);
	resource_format_po.unref();

	ResourceSaver::remove_resource_format_saver(resource_format_saver_crypto);
	resource_format_saver_crypto.unref();
	ResourceLoader::remove_resource_format_loader(resource_format_loader_crypto);
	resource_format_loader_crypto.unref();

	if (ip) {
		memdelete(ip);
	}

	ResourceLoader::finalize();

	ClassDB::cleanup_defaults();
	ObjectDB::cleanup();

	Variant::unregister_types();

	unregister_global_constants();

	ClassDB::cleanup();
	ResourceCache::clear();
	CoreStringNames::free();
	StringName::cleanup();
}
