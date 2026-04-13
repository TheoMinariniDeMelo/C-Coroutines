
local file_table<const> = {
    ["Linux x86_64"] = {"elf64", ".o", "x86_64"},
    ["Linux x86_32"] = {"elf32", ".o", "x86_32"},
    ["Windows AMD64"] = {"exe64", ".obj", "x86_64"},
    ["Windows x86"] = {"exe32", ".obj", "x86_32"},
}
local obj_dir<const> = "./obj"
local src_dir<const> = "./src"
local cc<const> = "gcc"
local cflags<const> = "-03"
local asm<const> = "nasm"

function getOS()
    -- ask LuaJIT first
    if jit then
        return jit.os
    end

    -- Unix, Linux variants
    local fh,err = assert(io.popen("uname -o 2>/dev/null","r"))
    if fh then
        osname = fh:read()
    end

    return osname or "Windows"
end

local function getAsmInfo()
    local os = getOS();
    local arch
    if os == "Windows" then
        local f = io.popen("echo %PROCESSOR_ARCHITECTURE%")
        if f == nil then
            return nil
        end
        local result = f:read("*l")

        arch = file_table["Windows "..result]
        f:close()
    else
        local f = io.popen("uname -m")
        if f == nil then
            return nil
        end
        local result = f:read("*l")
        f:close()

        arch = file_table["Linux "..result]
    end
    return arch
end

local function clean()
    os.execute("rm -rf "..obj_dir)
end

local function build(arch)
    clean()
    os.execute("mkdir "..obj_dir)
    local arch = getAsmInfo()
    local status = os.execute(asm.."f "..arch[1].." "..src_dir.."/coroutine-"..arch[3]..".asm".." -o coro"..arch[2])
    if status != 0 then 
        return nil
    end

    
end


build()
