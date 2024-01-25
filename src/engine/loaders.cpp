#define TINYOBJLOADER_IMPLEMENTATION
#define TINYPLY_IMPLEMENTATION
#include "loaders.h"

bool loaders::load_OBJ(Mesh *const mesh, bool overrideGeometry, const char *fileName, bool importMaterials, bool calculateTangents)
{
    // Preparing output
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileName, importMaterials ? nullptr : nullptr);

    // Check for errors
    if (!warn.empty())
    {
        DEBUG_LOG("WARN: " + warn);
    }
    if (!err.empty())
    {
        ERR_LOG(err);
        DEBUG_LOG("ERROR: Couldn't load mesh");
        return false;
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<Vertex, unsigned int> uniqueVertices;

    size_t shape_id = 0;
    // attrib.
    for (const tinyobj::shape_t &shape : shapes)
    {
        if (!shape.mesh.indices.empty())
            // IS INDEXED
            for (const tinyobj::index_t &index : shape.mesh.indices)
            {
                Vertex vertex = {};

                // positionition and color
                if (index.vertex_index >= 0)
                {
                    vertex.position.x = attrib.vertices[3 * index.vertex_index + 0];
                    vertex.position.y = attrib.vertices[3 * index.vertex_index + 1];
                    vertex.position.z = attrib.vertices[3 * index.vertex_index + 2];

                    vertex.color.r = attrib.colors[3 + index.vertex_index + 0];
                    vertex.color.g = attrib.colors[3 + index.vertex_index + 1];
                    vertex.color.b = attrib.colors[3 + index.vertex_index + 2];
                }
                // Normal
                if (index.normal_index >= 0)
                {
                    vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
                    vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
                    vertex.normal.z = attrib.normals[3 * index.normal_index + 2];
                }

                if (calculateTangents)
                {

                    // vertex.tangent = utils::get_tangent_gram_smidt(vertex.position,
                    //                                                vertices[indices[indices.size() - 2]].position,
                    //                                                vertices[indices[indices.size() - 1]].position,
                    //                                                vertex.uv,
                    //                                                vertices[indices[indices.size() - 2]].uv,
                    //                                                vertices[indices[indices.size() - 1]].uv,
                    //                                                vertex.tangent);
                }
                else
                {
                    vertex.tangent = {0.0, 0.0, 0.0};
                }

                // UV
                if (index.texcoord_index >= 0)
                {
                    vertex.uv.x = attrib.texcoords[2 * index.texcoord_index + 0];
                    vertex.uv.y = attrib.texcoords[2 * index.texcoord_index + 1];
                }

                // Check if the vertex is already in the map
                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<unsigned int>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        else
            // NOT INDEXED
            for (size_t i = 0; i < shape.mesh.num_face_vertices.size(); i++)
            {
                for (size_t j = 0; j < shape.mesh.num_face_vertices[i]; j++)
                {
                    Vertex vertex{};
                    size_t vertex_index = shape.mesh.indices[i * shape.mesh.num_face_vertices[i] + j].vertex_index;
                    // position
                    if (!attrib.vertices.empty())
                    {
                        vertex.position.x = attrib.vertices[3 * vertex_index + 0];
                        vertex.position.y = attrib.vertices[3 * vertex_index + 1];
                        vertex.position.z = attrib.vertices[3 * vertex_index + 2];
                    }
                    // Normals
                    if (!attrib.normals.empty())
                    {
                        vertex.normal.x = attrib.normals[3 * vertex_index + 0];
                        vertex.normal.y = attrib.normals[3 * vertex_index + 1];
                        vertex.normal.z = attrib.normals[3 * vertex_index + 2];
                    }
                    // Tangents
                    if (calculateTangents)
                    {
                        // vertex.tangent = utils::get_tangent_gram_smidt(vertex.position,
                        //                                                vertices[indices[indices.size() - 2]].position,
                        //                                                vertices[indices[indices.size() - 1]].position,
                        //                                                vertex.uv,
                        //                                                vertices[indices[indices.size() - 2]].uv,
                        //                                                vertices[indices[indices.size() - 1]].uv,
                        //                                                vertex.tangent);
                    }
                    else
                    {
                        vertex.tangent = {0.0, 0.0, 0.0};
                    }
                    // UV
                    if (!attrib.texcoords.empty())
                    {
                        vertex.uv.x = attrib.texcoords[2 * vertex_index + 0];
                        vertex.uv.y = attrib.texcoords[2 * vertex_index + 1];
                    }
                    // COLORS
                    if (!attrib.colors.empty())
                    {
                        vertex.color.r = attrib.colors[3 * vertex_index + 0];
                        vertex.color.g = attrib.colors[3 * vertex_index + 1];
                        vertex.color.b = attrib.colors[3 * vertex_index + 2];
                    }

                    vertices.push_back(vertex);
                }
            }

        Geometry g;
        g.vertices = vertices;
        g.indices = indices;
        mesh->set_geometry(g);

        shape_id++;
    }
    return true;
}

bool loaders::load_PLY(Mesh *const mesh, bool overrideGeometry, const char *fileName, bool preload, bool verbose, bool calculateTangents)
{

    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t> byte_buffer;
    std::string filePath = fileName;
    try
    {
        // For most files < 1gb, pre-loading the entire file upfront and wrapping it into a
        // stream is a net win for parsing speed, about 40% faster.
        if (preload)
        {
            byte_buffer = utils::read_file_binary(filePath);
            file_stream.reset(new utils::memory_stream((char *)byte_buffer.data(), byte_buffer.size()));
        }
        else
        {
            file_stream.reset(new std::ifstream(filePath, std::ios::binary));
        }

        if (!file_stream || file_stream->fail())
            throw std::runtime_error("file_stream failed to open " + filePath);

        file_stream->seekg(0, std::ios::end);
        const float size_mb = file_stream->tellg() * float(1e-6);
        file_stream->seekg(0, std::ios::beg);

        tinyply::PlyFile file;
        file.parse_header(*file_stream);

        if (verbose)
        {
            std::cout << "\t[ply_header] Type: " << (file.is_binary_file() ? "binary" : "ascii") << std::endl;
            for (const auto &c : file.get_comments())
                std::cout << "\t[ply_header] Comment: " << c << std::endl;
            for (const auto &c : file.get_info())
                std::cout << "\t[ply_header] Info: " << c << std::endl;

            for (const auto &e : file.get_elements())
            {
                std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
                for (const auto &p : e.properties)
                {
                    std::cout << "\t[ply_header] \tproperty: " << p.name << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
                    if (p.isList)
                        std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
                    std::cout << std::endl;
                }
            }
        }
        // Because most people have their own mesh types, tinyply treats parsed data as structured/typed byte buffers.
        std::shared_ptr<tinyply::PlyData> positions, normals, colors, texcoords, faces, tripstrip;

        // // The header information can be used to programmatically extract properties on elements
        // // known to exist in the header prior to reading the data. For brevity of this sample, properties
        // // like vertex position are hard-coded:
        try
        {
            positions = file.request_properties_from_element("vertex", {"x", "y", "z"});
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            normals = file.request_properties_from_element("vertex", {"nx", "ny", "nz"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"red", "green", "blue", "alpha"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"r", "g", "b", "a"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            texcoords = file.request_properties_from_element("vertex", {"u", "v"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Providing a list size hint (the last argument) is a 2x performance improvement. If you have
        // arbitrary ply files, it is best to leave this 0.
        try
        {
            faces = file.request_properties_from_element("face", {"vertex_indices"}, 3);
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Tristrips must always be read with a 0 list size hint (unless you know exactly how many elements
        // are specifically in the file, which is unlikely);
        try
        {
            tripstrip = file.request_properties_from_element("tristrips", {"vertex_indices"}, 0);
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        if (verbose)
        {
            utils::ManualTimer readTimer;

            readTimer.start();
            file.read(*file_stream);
            readTimer.stop();

            const float parsingTime = static_cast<float>(readTimer.get()) / 1000.f;
            std::cout << "\tparsing " << size_mb << "mb in " << parsingTime << " seconds [" << (size_mb / parsingTime) << " MBps]" << std::endl;

            if (positions)
                std::cout << "\tRead " << positions->count << " total vertices " << std::endl;
            if (normals)
                std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
            if (colors)
                std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
            if (texcoords)
                std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
            if (faces)
                std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;
            if (tripstrip)
                std::cout << "\tRead " << (tripstrip->buffer.size_bytes() / tinyply::PropertyTable[tripstrip->t].stride) << " total indices (tristrip) " << std::endl;
        }

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        if (positions)
        {
            const float *posData = reinterpret_cast<const float *>(positions->buffer.get());
            for (size_t i = 0; i < positions->count; ++i)
            {
                float x = posData[i * 3];
                float y = posData[i * 3 + 1];
                float z = posData[i * 3 + 2];

                // Assuming Vertex has a constructor that takes position attributes
                vertices.push_back({{x, y, z}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}); // You can set color and other attributes as needed
            }
        }
        const int *facesData = reinterpret_cast<const int *>(faces->buffer.get());
        for (size_t i = 0; i < faces->count; i += 3)
        {
            // Assuming faces are triangles, so we extract the vertex indices
            unsigned int vertexIndex1 = static_cast<unsigned int>(facesData[i]);
            unsigned int vertexIndex2 = static_cast<unsigned int>(facesData[i + 1]);
            unsigned int vertexIndex3 = static_cast<unsigned int>(facesData[i + 2]);

            indices.push_back(vertexIndex1);
            indices.push_back(vertexIndex2);
            indices.push_back(vertexIndex3);
        }

        Geometry g;
        g.vertices = vertices;
        // g.indices = indices;
        mesh->set_geometry(g);

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }

    return false;
}

bool loaders::load_NeuralHair(Mesh *const mesh, bool overrideGeometry, const char *fileName, bool preload, bool verbose, bool calculateTangents)
{
    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t> byte_buffer;
    std::string filePath = fileName;
    try
    {
        // For most files < 1gb, pre-loading the entire file upfront and wrapping it into a
        // stream is a net win for parsing speed, about 40% faster.
        if (preload)
        {
            byte_buffer = utils::read_file_binary(filePath);
            file_stream.reset(new utils::memory_stream((char *)byte_buffer.data(), byte_buffer.size()));
        }
        else
        {
            file_stream.reset(new std::ifstream(filePath, std::ios::binary));
        }

        if (!file_stream || file_stream->fail())
            throw std::runtime_error("file_stream failed to open " + filePath);

        file_stream->seekg(0, std::ios::end);
        const float size_mb = file_stream->tellg() * float(1e-6);
        file_stream->seekg(0, std::ios::beg);

        tinyply::PlyFile file;
        file.parse_header(*file_stream);

        if (verbose)
        {
            std::cout << "\t[ply_header] Type: " << (file.is_binary_file() ? "binary" : "ascii") << std::endl;
            for (const auto &c : file.get_comments())
                std::cout << "\t[ply_header] Comment: " << c << std::endl;
            for (const auto &c : file.get_info())
                std::cout << "\t[ply_header] Info: " << c << std::endl;

            for (const auto &e : file.get_elements())
            {
                std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
                for (const auto &p : e.properties)
                {
                    std::cout << "\t[ply_header] \tproperty: " << p.name << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
                    if (p.isList)
                        std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
                    std::cout << std::endl;
                }
            }
        }
        // Because most people have their own mesh types, tinyply treats parsed data as structured/typed byte buffers.
        std::shared_ptr<tinyply::PlyData> positions, normals, colors, texcoords, faces, tripstrip;

        // // The header information can be used to programmatically extract properties on elements
        // // known to exist in the header prior to reading the data. For brevity of this sample, properties
        // // like vertex position are hard-coded:
        try
        {
            positions = file.request_properties_from_element("vertex", {"x", "y", "z"});
        }
        catch (const std::exception &e)
        {
            std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            normals = file.request_properties_from_element("vertex", {"nx", "ny", "nz"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"red", "green", "blue", "alpha"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            colors = file.request_properties_from_element("vertex", {"r", "g", "b", "a"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        try
        {
            texcoords = file.request_properties_from_element("vertex", {"u", "v"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Providing a list size hint (the last argument) is a 2x performance improvement. If you have
        // arbitrary ply files, it is best to leave this 0.
        try
        {
            faces = file.request_properties_from_element("face", {"vertex_indices"}, 3);
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        // Tristrips must always be read with a 0 list size hint (unless you know exactly how many elements
        // are specifically in the file, which is unlikely);
        try
        {
            tripstrip = file.request_properties_from_element("tristrips", {"vertex_indices"}, 0);
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }

        if (verbose)
        {
            utils::ManualTimer readTimer;

            readTimer.start();
            file.read(*file_stream);
            readTimer.stop();

            const float parsingTime = static_cast<float>(readTimer.get()) / 1000.f;
            std::cout << "\tparsing " << size_mb << "mb in " << parsingTime << " seconds [" << (size_mb / parsingTime) << " MBps]" << std::endl;

            if (positions)
                std::cout << "\tRead " << positions->count << " total vertices " << std::endl;
            if (normals)
                std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
            if (colors)
                std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
            if (texcoords)
                std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
            if (faces)
                std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;
            if (tripstrip)
                std::cout << "\tRead " << (tripstrip->buffer.size_bytes() / tinyply::PropertyTable[tripstrip->t].stride) << " total indices (tristrip) " << std::endl;
        }

        std::vector<Vertex> vertices;
        // std::vector<unsigned int> indices;

        if (positions)
        {
            const float *posData = reinterpret_cast<const float *>(positions->buffer.get());
            const unsigned char *colorData = reinterpret_cast<const unsigned char *>(colors->buffer.get());
            for (size_t i = 0; i < positions->count; ++i)
            {
                float x = posData[i * 3];
                float y = posData[i * 3 + 1];
                float z = posData[i * 3 + 2];
                float r = (float)colorData[i * 4];
                float g = (float)colorData[i * 4 + 1];
                float b = (float)colorData[i * 4 + 2];
                float a = (float)colorData[i * 4 + 3];

                // Assuming Vertex has a constructor that takes position attributes
                vertices.push_back({{x, y, z}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {r/255, g/255, b/255}}); // You can set color and other attributes as needed
            }
        }

        // const int *facesData = reinterpret_cast<const int *>(faces->buffer.get());
        // for (size_t i = 0; i < faces->count; i += 3)
        // {
        //     // Assuming faces are triangles, so we extract the vertex indices
        //     unsigned int vertexIndex1 = static_cast<unsigned int>(facesData[i]);
        //     unsigned int vertexIndex2 = static_cast<unsigned int>(facesData[i + 1]);
        //     unsigned int vertexIndex3 = static_cast<unsigned int>(facesData[i + 2]);

        //     indices.push_back(vertexIndex1);
        //     indices.push_back(vertexIndex2);
        //     indices.push_back(vertexIndex3);
        // }

        Geometry g;
        g.vertices = vertices;
        // g.indices = indices;
        mesh->set_geometry(g);

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }

    return false;
}
