function checks()
    @ccall "$(pwd())/build/libizmirvmlib.so".izmirvm_initialize()::Cvoid
    @ccall "$(pwd())/build/libizmirvmlib.so".izmirvm_finalize()::Cvoid
end


checks()