#include <config.h>
#include "Map.h"
#include "render/physics.h"


Map::Map()
{
}

Map::Map(float scaleMultiplier, float spacing)
{
    this->gridSizeX = 0;
    this->gridSizeZ = 0;
	this->scaleMultiplier = scaleMultiplier;
	this->spacing = spacing;

}

void Map::readMapFromFile(std::string filePath)
{
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "Error: Cannot open file!" << std::endl;
		return;
	}

	std::string line = "";
	while (std::getline(file, line))
	{
		map.push_back(std::vector<char>(line.begin(), line.end()));
	}
	file.close();
}



void Map::settingUpMap1()
{

    // Set grid dimensions
    gridSizeZ = map.size();
    gridSizeX = map[0].size();

    // Pre-calculate half grid dimensions for positioning
    const float halfX = gridSizeX / 2.0f;
    const float halfZ = gridSizeZ / 2.0f;
    int straightCount = 0;

    // Process each cell in the map in a single pass
    for (int z = 0; z < gridSizeZ; z++)
    {

        for (int x = 0; x < gridSizeX; x++)
        {
            Object obj;
            obj.objType = objectType::EMPTY; // Default to empty

            // Calculate position
            glm::vec3 translation = glm::vec3(
                (x - halfX) * spacing,
                0.0f,
                (z - halfZ) * spacing
            );
            glm::vec3 scale = glm::vec3(scaleMultiplier);

            // Initialize transform with common operations
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                * glm::scale(glm::mat4(1.0f), scale);

            // Handle each object type
            switch (map[z][x])
            {
            case 'R': // Round corner
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/round-corner-b.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/round-corner-b.glb");
                obj.objType = objectType::ROUND_CORNER;
                transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
                obj.transform = transform;
                break;

            case 'S': // Straight
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/straight.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/straight.glb");
                obj.objType = objectType::STRAIGHT; 
                
                if (straightCount <= 3)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    straightCount += 1;
                }
                else
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    straightCount += 1;
                }
                
                
                break;

            case 'H': // Hole square
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/hole-square.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/hole-square.glb");
                obj.objType = objectType::HOLE_SQUARE;
                transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                obj.transform = transform;
                holePos = transform[3] + glm::vec4(0.0f, 0.5f, 0.0f, 0.0f);

                break;

            case 'C': // Castle
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/castle.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/castle.glb");
                obj.objType = objectType::CASTLE;
                obj.transform = transform;
                break;

            case 'E': // End
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/end.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/end.glb");
                obj.objType = objectType::END;
                obj.transform = transform;

                //setting spawnpoint
                this->spawnPoint = transform[3]+glm::vec4(0,0.5f,0,0);

                break;

            case 'F': // Flag
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/flag-red.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/flag-red.glb");
                
                obj.objType = objectType::FLAG;
                obj.transform = transform;
                obj.transform[3] = glm::vec4(holePos - glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);
                break;

            case '0': // Empty
                obj.model = Render::ModelId(); 
                obj.objType = objectType::EMPTY;
                obj.transform = transform;
                break;

            default:
                std::cerr << "Warning: Unknown map character '" << map[z][x] << "' at position ("
                    << x << ", " << z << ")" << std::endl;
                continue; // Skip this object
            }

           

            // Only create colliders for non-empty objects
            if (obj.objType != objectType::EMPTY) 
            {
               
                if (obj.model) 
                {
                    obj.colliderID = Physics::CreateCollider(obj.colliderMesh, obj.transform,2);
                    
                }
                else 
                {
                    std::cerr << "Error: Failed to load assets for object at ("
                        << x << ", " << z << ")" << std::endl;
                }
            }

            objects.push_back(obj);

        }
    }
}

void Map::settingUpMap2()
{
    // Set grid dimensions
    gridSizeZ = map.size();
    gridSizeX = map[0].size();

    // Pre-calculate half grid dimensions for positioning
    const float halfX = gridSizeX / 2.0f;
    const float halfZ = gridSizeZ / 2.0f;
    int straightCount = 0;
    int roundCornerCount = 0;

    // Process each cell in the map in a single pass
    for (int z = 0; z < gridSizeZ; z++)
    {

        for (int x = 0; x < gridSizeX; x++)
        {
            Object obj;
            obj.objType = objectType::EMPTY; // Default to empty

            // Calculate position
            glm::vec3 translation = glm::vec3(
                (x - halfX) * spacing,
                0.0f,
                (z - halfZ) * spacing
            );
            glm::vec3 scale = glm::vec3(scaleMultiplier);

            // Initialize transform with common operations
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                * glm::scale(glm::mat4(1.0f), scale);

            // Handle each object type
            switch (map[z][x])
            {
            case 'R': // Round corner
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/round-corner-b.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/round-corner-b.glb");
                obj.objType = objectType::ROUND_CORNER;
                if (roundCornerCount < 1)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    roundCornerCount += 1;
                }
                else
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    roundCornerCount += 1;
                }
                break;

            case 'Q': // SQUARE corner
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/corner.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/corner.glb");
                obj.objType = objectType::ROUND_CORNER;
                transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
                obj.transform = transform;
                break;

            case 'S': // Straight
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/straight.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/straight.glb");
                obj.objType = objectType::STRAIGHT;
                if (straightCount == 0)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    straightCount += 1;
                }
                else if(straightCount == 1)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    straightCount += 1;
                }
                else if (straightCount == 2)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    straightCount += 1;
                }
                break;

            case 'H': // Hole square
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/hole-square.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/hole-square.glb");
                obj.objType = objectType::HOLE_SQUARE;
                transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0, 1, 0));
                obj.transform = transform; 

                //savingData
                holePos = transform[3] + glm::vec4(0.0f, 0.5f, 0.0f, 0.0f);
                break;

            case 'N': // Narrow
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/narrow-square.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/narrow-square.glb");
                obj.objType = objectType::NARROW_SQUARE;
                transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                obj.transform = transform;
                break;

            case 'E': // End
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/end.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/end.glb");
                obj.objType = objectType::END;
                obj.transform = transform;

                //setting spawnpoint
                this->spawnPoint = transform[3] + glm::vec4(0, 0.5f, 0, 0);
                break;

            case 'T': //tunnel
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/tunnel-double.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/tunnel-double.glb");
                obj.objType = objectType::TUNNEL_DOUBLE;
                transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                obj.transform = transform;
                break;

            case 'F': // Flag
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/flag-red.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/flag-red.glb");
                
                obj.objType = objectType::FLAG;
                obj.transform = transform;
                obj.transform[3] = glm::vec4(holePos - glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);
                break;

            case 'W':
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/split.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/split.glb");
                obj.objType = objectType::SPLIT;
                transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                obj.transform = transform;
                break;

            case '0': // Empty
                obj.model = Render::ModelId();
                obj.objType = objectType::EMPTY;
                obj.transform = transform;
                break;

            default:
                std::cerr << "Warning: Unknown map character '" << map[z][x] << "' at position ("
                    << x << ", " << z << ")" << std::endl;
                continue; // Skip this object
            }


            // Only create colliders for non-empty objects
            if (obj.objType != objectType::EMPTY)
            {
                if (obj.model) {
                    obj.colliderID = Physics::CreateCollider(obj.colliderMesh, obj.transform,2);
                }
                else {
                    std::cerr << "Error: Failed to load assets for object at ("
                        << x << ", " << z << ")" << std::endl;
                }
            }

            objects.push_back(obj);
        }
    }
}

void Map::settingUpMap3()
{
    // Set grid dimensions
    gridSizeZ = map.size();
    gridSizeX = map[0].size();

    // Pre-calculate half grid dimensions for positioning
    const float halfX = gridSizeX / 2.0f;
    const float halfZ = gridSizeZ / 2.0f;

    //counting on which one of the same tile should be rotated
    int straightCount = 0;
    int roundCornerCount = 0;
    int splitCount = 0;

    // Process each cell in the map in a single pass
    for (int z = 0; z < gridSizeZ; z++)
    {

        for (int x = 0; x < gridSizeX; x++)
        {
            Object obj;
            obj.objType = objectType::EMPTY; // Default to empty

            // Calculate position
            glm::vec3 translation = glm::vec3(
                (x - halfX) * spacing,
                0.0f,
                (z - halfZ) * spacing
            );
            glm::vec3 scale = glm::vec3(scaleMultiplier);

            // Initialize transform with common operations
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                * glm::scale(glm::mat4(1.0f), scale);

            // Handle each object type
            switch (map[z][x])
            {
            case 'R': // Round corner
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/round-corner-b.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/round-corner-b.glb");
                obj.objType = objectType::ROUND_CORNER;
                if (roundCornerCount == 0)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    roundCornerCount += 1;
                }
                else if(roundCornerCount == 1)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    roundCornerCount += 1;
                }
                else if (roundCornerCount == 2)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    roundCornerCount += 1;
                }
                else if (roundCornerCount == 3)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    roundCornerCount += 1;
                }
                break;

            case 'K': // SQUARE corner
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/wall-right.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/wall-right.glb");
                obj.objType = objectType::WALL_RIGHT;
                
                obj.transform = transform;
                break;

            case 'H': // Hole square
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/hole-square.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/hole-square.glb");
                obj.objType = objectType::HOLE_SQUARE;
                transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                obj.transform = transform;

                holePos = transform[3] + glm::vec4(0.0f, 0.5f, 0.0f, 0.0f);
                break;

            case 'E': // End
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/end.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/end.glb");
                obj.objType = objectType::END;
                transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0, 1, 0));
                obj.transform = transform;

                //setting spawnpoint
                this->spawnPoint = transform[3] + glm::vec4(0, 0.5f, 0, 0);
                break;

            case 'C': //tunnel
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/castle.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/castle.glb");
                obj.objType = objectType::CASTLE;
               // transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                obj.transform = transform;
                break;
            case 'W':
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/split.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/split.glb");
                obj.objType = objectType::SPLIT;
                if (splitCount == 0)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    splitCount += 1;
                }
                else if (splitCount == 1)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    splitCount += 1;
                }
                else if (splitCount == 2)
                {
                    transform = transform * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
                    obj.transform = transform;
                    splitCount += 1;
                    }

                obj.transform = transform;
                break;

            case 'L':
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/wall-left.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/wall-left.glb");
                obj.objType = objectType::WALL_LEFT;
                obj.transform = transform;
                break;

            case 'F': // Flag
                obj.model = Render::LoadModel("assets/Minigolf/Models/GLB format/flag-red.glb");
                obj.colliderMesh = Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/flag-red.glb");
               
                obj.objType = objectType::FLAG;
                obj.transform = transform;
                obj.transform[3] = glm::vec4(holePos - glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);
                break;

            case '0': // Empty
                obj.model = Render::ModelId();
                obj.objType = objectType::EMPTY;
                obj.transform = transform;

                break;

            default:
                std::cerr << "Warning: Unknown map character '" << map[z][x] << "' at position ("
                    << x << ", " << z << ")" << std::endl;
                continue; // Skip this object
            }


            // Only create colliders for non-empty objects
            if (obj.objType != objectType::EMPTY)
            {
                if (obj.model) 
                {
                    obj.colliderID = Physics::CreateCollider(obj.colliderMesh, obj.transform,2);
                }
                else 
                {
                    std::cerr << "Error: Failed to load assets for object at ("
                        << x << ", " << z << ")" << std::endl;
                }
            }

            objects.push_back(obj);
        }
    }
}
