#include <Graphics/Scene/Scene.h>

#include <Graphics/Scene/Camera.h>

namespace cyd
{
Scene::Scene() {}

// void Scene::loadScene()
// {
//    _vertices.clear();
//    _indices.clear();

//    for(const auto& mesh : _meshes)
//    {

//    }
// }

// void Scene::loadMeshRAM()
// {
//    tinyobj::attrib_t attrib;
//    std::vector<tinyobj::shape_t> shapes;
//    std::vector<tinyobj::material_t> materials;
//    std::string warn, err;

//    bool res = tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, _path.c_str() );
//    CYDASSERT( res && "Model loading failed" );

//    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

//    for( const auto& shape : shapes )
//    {
//       for( const auto& index : shape.mesh.indices )
//       {
//          Vertex vertex = {};

//          vertex.pos = {
//              attrib.vertices[3 * index.vertex_index + 0],
//              attrib.vertices[3 * index.vertex_index + 1],
//              attrib.vertices[3 * index.vertex_index + 2] };

//          vertex.normal = {
//              attrib.normals[3 * index.normal_index + 0],
//              attrib.normals[3 * index.normal_index + 1],
//              attrib.normals[3 * index.normal_index + 2] };

//          vertex.uv = {
//              attrib.texcoords[2 * index.texcoord_index + 0],
//              1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

//          vertex.col = { 1.0f, 1.0f, 1.0f, 1.0f };

//          if( uniqueVertices.count( vertex ) == 0 )
//          {
//             uniqueVertices[vertex] = static_cast<uint32_t>( _vertices.size() );
//             _vertices.push_back( vertex );
//          }

//          _indices.push_back( uniqueVertices[vertex] );
//       }
//    }
// }

Scene::~Scene() {}
}