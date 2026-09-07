import struct

def generate_valid_test_mdl(file_path="test_prop.mdl"):
    # Pre-allocate a safe 256-byte binary data block
    mdl_buffer = bytearray(256)
    
    # 1. Write the Magic ID token 'IDST' (0x54534449) at offset 0
    struct.pack_into("<I", mdl_buffer, 0, 0x54534449)
    
    # 2. Write the Source Studio version identifier (48) at offset 4
    struct.pack_into("<I", mdl_buffer, 4, 48)
    
    # 3. Embed an internal model name string starting at offset 12
    model_name = b"models/props_junk/test_crate.mdl"
    mdl_buffer[12:12+len(model_name)] = model_name
    
    # 4. Inject explicit sample metrics at your C++ parser's specific layout seek offsets
    struct.pack_into("<i", mdl_buffer, 164, 8)   # 8 Bone links -> will alter wireframe scale
    struct.pack_into("<i", mdl_buffer, 204, 1140) # 1,140 Triangles density
    
    # Write the compiled mock container out to disk
    with open(file_path, "wb") as f:
        file_path_written = f.write(mdl_buffer)
    print(f"Successfully generated a valid tool-test model at: {file_path}")

if __name__ == "__main__":
    generate_valid_test_mdl()
