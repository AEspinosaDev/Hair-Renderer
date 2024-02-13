#define TINYOBJLOADER_IMPLEMENTATION
#define TINYPLY_IMPLEMENTATION
#include "loaders.h"

GLIB_NAMESPACE_BEGIN

void loaders::load_OBJ(Mesh *const mesh, const char *fileName, bool importMaterials, bool calculateTangents)
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
        return;
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

                vertex.tangent = {0.0, 0.0, 0.0};

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

                    vertex.tangent = {0.0, 0.0, 0.0};

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
}

void loaders::load_PLY(Mesh *const mesh, const char *fileName, bool preload, bool verbose, bool calculateTangents)
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
        vertices.reserve(positions->count);
        std::vector<unsigned int> indices;
        indices.reserve(faces->count * 3);

        if (positions)
        {
            const float *posData = reinterpret_cast<const float *>(positions->buffer.get());
            const float *normalData;
            if (normals)
                normalData = reinterpret_cast<const float *>(normals->buffer.get());

            for (size_t i = 0; i < positions->count; i++)
            {
                float x = posData[i * 3];
                float y = posData[i * 3 + 1];
                float z = posData[i * 3 + 2];

                float nx = normals ? normalData[i * 3] : 0.0f;
                float ny = normals ? normalData[i * 3 + 1] : 0.0f;
                float nz = normals ? normalData[i * 3 + 2] : 0.0f;

                // Assuming Vertex has a constructor that takes position attributes
                vertices.push_back({{x, y, z}, {nx, ny, nz}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}); // You can set color and other attributes as needed
            }
        }
        unsigned *facesData = reinterpret_cast<unsigned *>(faces->buffer.get());
        for (size_t i = 0; i < faces->count; ++i)
        {

            // Assuming faces are triangles, so we extract the vertex indices
            unsigned int vertexIndex1 = static_cast<unsigned int>(facesData[i]);
            unsigned int vertexIndex2 = static_cast<unsigned int>(facesData[i + 1]);
            unsigned int vertexIndex3 = static_cast<unsigned int>(facesData[i + 2]);

            indices.push_back(facesData[3 * i]);
            indices.push_back(facesData[3 * i + 1]);
            indices.push_back(facesData[3 * i + 2]);
        }

        Geometry g;
        g.vertices = vertices;
        g.indices = indices;
        mesh->set_geometry(g);

        return;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
}

void loaders::load_neural_hair(Mesh *const mesh, const char *fileName, bool preload, bool verbose, bool calculateTangents)
{
    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t> byte_buffer;
    std::string filePath = fileName;
    try
    {
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
        std::shared_ptr<tinyply::PlyData> positions, colors;
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
            colors = file.request_properties_from_element("vertex", {"red", "green", "blue", "alpha"});
        }
        catch (const std::exception &e)
        {
            if (verbose)
                std::cerr << "tinyply exception: " << e.what() << std::endl;
        }
        utils::ManualTimer readTimer;
        readTimer.start();
        file.read(*file_stream);
        readTimer.stop();

        if (verbose)
        {
            const float parsingTime = static_cast<float>(readTimer.get()) / 1000.f;
            std::cout << "\tparsing " << size_mb << "mb in " << parsingTime << " seconds [" << (size_mb / parsingTime) << " MBps]" << std::endl;
            if (positions)
                std::cout << "\tRead " << positions->count << " total vertices " << std::endl;
            if (colors)
                std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
        }

        std::vector<Vertex> vertices;
        vertices.reserve(positions->count);
        std::vector<unsigned int> indices;
        std::vector<unsigned int> rootsIndices;

        if (positions)
        {
            rootsIndices.push_back(0); // First index is certainly a root
            const float *posData = reinterpret_cast<const float *>(positions->buffer.get());
            const unsigned char *colorData = reinterpret_cast<const unsigned char *>(colors->buffer.get());
            for (size_t i = 0; i < positions->count - 1; i++)
            {
                float x = posData[i * 3];
                float y = posData[i * 3 + 1];
                float z = posData[i * 3 + 2];

                // Generate hair tangents
                glm::vec3 pos = {x, y, z};
                float nextX = posData[(i + 1) * 3];
                float nextY = posData[(i + 1) * 3 + 1];
                float nextZ = posData[(i + 1) * 3 + 2];
                glm::vec3 nextPos = {nextX, nextY, nextZ};
                glm::vec3 tangent = glm::normalize(nextPos - pos);

                float r = (float)colorData[i * 4];
                float g = (float)colorData[i * 4 + 1];
                float b = (float)colorData[i * 4 + 2];

                float nextR = (float)colorData[(i + 1) * 4];
                float nextG = (float)colorData[(i + 1) * 4 + 1];
                float nextB = (float)colorData[(i + 1) * 4 + 2];

                vertices.push_back({pos, {0.0f, 0.0f, 0.0f}, tangent, {0.0f, 0.0f}, {r / 255, g / 255, b / 255}});
                if (i == positions->count - 2)
                    vertices.push_back({nextPos, {0.0f, 0.0f, 0.0f}, tangent, {0.0f, 0.0f}, {r / 255, g / 255, b / 255}});

                if (r == nextR && g == nextG && b == nextB)
                {
                    indices.push_back(i);
                    indices.push_back(i + 1);
                }
                else
                {
                    rootsIndices.push_back(i + 1);
                }
            }
        }

        Geometry g;
        g.vertices = vertices;
        g.indices = indices;
        augment_strands_density(g, rootsIndices);
        mesh->set_geometry(g);

        return;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
}
void loaders::augment_strands_density(Geometry &geom, std::vector<unsigned int> &roots, const int kNeighbours, const int strandsPerNeighbourhood)
{
    struct Root
    {
        unsigned int id;
        float dist;
    };
    struct Neighbourhood
    {
        std::vector<unsigned int> ids;
        float meanDist;
    };

    // According strands are the same length.
    // Neural haircut asures it
    const unsigned int ROOT_LENGTH = roots[1] - roots[0] - 1;

    std::vector<Neighbourhood> nearestNeighbours;
    nearestNeighbours.resize(roots.size());
    float min = std::numeric_limits<float>::max();
    float totalDistPerHood = 0.0f;

    // Neighbour adjacency list;
    for (size_t i = 0; i < roots.size(); i++)
    {
        std::vector<Root> rootsByDist;
        for (size_t j = 0; j < roots.size(); j++)
        {
            if (i != j)
            {
                float dist = glm::distance(geom.vertices[roots[i]].position, geom.vertices[roots[j]].position);
                Root r{roots[j], dist};
                rootsByDist.push_back(r);
            }
        }

        std::sort(rootsByDist.begin(), rootsByDist.end(), [](const Root &a, const Root &b)
                  { return a.dist < b.dist; });

        float totalDist = 0.0f;
        int count = 0;
        for (auto it = rootsByDist.begin(); it != rootsByDist.end() && count < kNeighbours; ++it, ++count)
        {
            Root r = *it;
            totalDist += r.dist;
            nearestNeighbours[i].ids.push_back(r.id);
        }
        nearestNeighbours[i].meanDist = totalDist / kNeighbours;
        totalDistPerHood += nearestNeighbours[i].meanDist;
    }

    float meanDistPerHood = totalDistPerHood / roots.size();
    float typicalDeviation = 0.0f;

    for (size_t i = 0; i < nearestNeighbours.size(); i++)
    {
        float deviation = nearestNeighbours[i].meanDist - meanDistPerHood;
        // int const HOOD_STRANDS = strandsPerNeighbourhood * -(1/(1+exp(-TDeviation)));
        // int HOOD_STRANDS = strandsPerNeighbourhood;
        int const HOOD_STRANDS = strandsPerNeighbourhood * nearestNeighbours[i].meanDist / meanDistPerHood;

        for (size_t s = 0; s < HOOD_STRANDS; s++)
        {

            std::vector<float> weights;
            weights.reserve(kNeighbours);
            float totalWeight = 0;

            // RANDON WEIGHT PER NEIGHBOUR
            for (size_t j = 0; j < kNeighbours; j++)
            {
                float w = ((float)rand()) / RAND_MAX;
                weights.push_back(w);
                totalWeight += w;
            }

            // CHOOSE RANDOM COLOR FOR DEBUG
            glm::vec3 color = {((float)rand()) / RAND_MAX, ((float)rand()) / RAND_MAX, ((float)rand()) / RAND_MAX};
            unsigned int currentIndex = geom.indices.back() + 1;

            // GROW NEW STRAND
            for (size_t j = 0; j < ROOT_LENGTH; j++)
            {
                // Mean of strands direction
                // float totalOrientation = 0.0f;
                // for (size_t k = 0; k < kNeighbours; k++)
                // {
                //     totalOrientation += glm::dot(geom.vertices[nearestNeighbours[i].ids[k] + j].tangent, geom.vertices[roots[i]].tangent);
                // }
                // float orientationMean = totalOrientation / kNeighbours;
                // Deviation, if deviation higher than treshold, weight 0
                // totalWeight = 0.0f;
                // for (size_t k = 0; k < kNeighbours; k++)
                // {
                //     if (glm::dot(geom.vertices[nearestNeighbours[i].ids[k] + j].tangent, geom.vertices[roots[i]].tangent) <= 0.0f)
                //         weights[k] = 0.0f;
                //     totalWeight += weights[k];
                // }

                glm::vec3 totalPos = glm::vec3(0.0f);
                for (size_t k = 0; k < kNeighbours; k++)
                {
                    totalPos += geom.vertices[nearestNeighbours[i].ids[k] + j].position * weights[k];
                }

                glm::vec3 newPos = totalPos / totalWeight;

                geom.vertices.push_back({newPos, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, color});
                if (j < ROOT_LENGTH - 1)
                {
                    geom.indices.push_back(currentIndex);
                    geom.indices.push_back(currentIndex + 1);
                    currentIndex++;
                }
            }
        }
    }
}
void loaders::load_cy_hair(Mesh *const mesh, const char *fileName)
{

#define HAIR_FILE_SEGMENTS_BIT 1
#define HAIR_FILE_POINTS_BIT 2
#define HAIR_FILE_THICKNESS_BIT 4
#define HAIR_FILE_TRANSPARENCY_BIT 8
#define HAIR_FILE_COLORS_BIT 16
#define HAIR_FILE_INFO_SIZE 88

    unsigned short *segments;
    float *points;
    float *thickness;
    float *transparency;
    float *colors;

    struct Header
    {
        char signature[4];        //!< This should be "HAIR"
        unsigned int hair_count;  //!< number of hair strands
        unsigned int point_count; //!< total number of points of all strands
        unsigned int arrays;      //!< bit array of data in the file

        unsigned int d_segments; //!< default number of segments of each strand
        float d_thickness;       //!< default thickness of hair strands
        float d_transparency;    //!< default transparency of hair strands
        float d_color[3];        //!< default color of hair strands

        char info[HAIR_FILE_INFO_SIZE]; //!< information about the file
    };

    Header header;

    header.signature[0] = 'H';
    header.signature[1] = 'A';
    header.signature[2] = 'I';
    header.signature[3] = 'R';
    header.hair_count = 0;
    header.point_count = 0;
    header.arrays = 0; // no arrays
    header.d_segments = 0;
    header.d_thickness = 1.0f;
    header.d_transparency = 0.0f;
    header.d_color[0] = 1.0f;
    header.d_color[1] = 1.0f;
    header.d_color[2] = 1.0f;
    memset(header.info, '\0', HAIR_FILE_INFO_SIZE);

    FILE *fp;
    fp = fopen(fileName, "rb");
    if (fp == nullptr)
        return;

    // read the header
    size_t headread = fread(&header, sizeof(Header), 1, fp);

    // Check if header is correctly read
    if (headread < 1)
        return;

    // Check if this is a hair file
    if (strncmp(header.signature, "HAIR", 4) != 0)
        return;

    // Read segments array
    if (header.arrays & HAIR_FILE_SEGMENTS_BIT)
    {
        segments = new unsigned short[header.hair_count];
        size_t readcount = fread(segments, sizeof(unsigned short), header.hair_count, fp);
        if (readcount < header.hair_count)
        {
            std::cerr << "Error reading segments" << std::endl;
            return;
        }
    }

    // Read points array
    if (header.arrays & HAIR_FILE_POINTS_BIT)
    {
        points = new float[header.point_count * 3];
        size_t readcount = fread(points, sizeof(float), header.point_count * 3, fp);
        if (readcount < header.point_count * 3)
        {
            std::cerr << "Error reading points" << std::endl;
            return;
        }
    }

    // Read thickness array
    if (header.arrays & HAIR_FILE_THICKNESS_BIT)
    {
        thickness = new float[header.point_count];
        size_t readcount = fread(thickness, sizeof(float), header.point_count, fp);
        if (readcount < header.point_count)
        {
            std::cerr << "Error reading thickness" << std::endl;
            return;
        }
    }

    // Read thickness array
    if (header.arrays & HAIR_FILE_TRANSPARENCY_BIT)
    {
        transparency = new float[header.point_count];
        size_t readcount = fread(transparency, sizeof(float), header.point_count, fp);
        if (readcount < header.point_count)
        {
            std::cerr << "Error reading alpha" << std::endl;
            return;
        }
    }

    // Read colors array
    if (header.arrays & HAIR_FILE_COLORS_BIT)
    {
        colors = new float[header.point_count * 3];
        size_t readcount = fread(colors, sizeof(float), header.point_count * 3, fp);
        if (readcount < header.point_count * 3)
        {
            std::cerr << "Error reading colors" << std::endl;
            return;
        }
    }

    fclose(fp);

    std::vector<Vertex> vertices;
    vertices.reserve(header.point_count * 3);
    std::vector<unsigned int> indices;

    size_t index = 0;
    size_t pointId = 0;
    for (size_t hair = 0; hair < header.hair_count; hair++)
    {
        glm::vec3 color = {((float)rand()) / RAND_MAX, ((float)rand()) / RAND_MAX, ((float)rand()) / RAND_MAX};
        size_t max_segments = segments ? segments[hair] : header.d_segments;
        for (size_t i = 0; i < max_segments; i++)
        {
            vertices.push_back({{points[pointId], points[pointId + 1], points[pointId + 2]}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, color});
            indices.push_back(index);
            indices.push_back(index + 1);
            index++;
            pointId += 3;
        }
        vertices.push_back({{points[pointId], points[pointId + 1], points[pointId + 2]}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, color});
        pointId += 3;
        index++;
    }

    Geometry g;
    g.vertices = vertices;
    g.indices = indices;
    mesh->set_geometry(g);
}
GLIB_NAMESPACE_END