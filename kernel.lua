local kernel = {}

function kernel.boot()
    print("kernel.lua boot() running")
    tb.write("Lua reached the kernel layer")

    return {
        name = "tbOS",
        version = "0.1.0",
        status = "lua-online",
    }
end

return kernel
