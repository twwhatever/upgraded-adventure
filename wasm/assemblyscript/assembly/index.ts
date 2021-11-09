// The entry file of your WebAssembly module.

import {log_buffer} from './imports'

export function foo(): i32 {
    return 42;
}

export function hi(pbuf: usize, size: usize): usize {
    let gbuf = log_buffer(pbuf, usize);
    let str = String.UTF8.decodeUnsafe(gbuf, 1024, true);
    let rv = String.UTF8.encode(`Hi, ${str}, from AssemblyScript!`, true);
    return changetype<usize>(rv);
}
