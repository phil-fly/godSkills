SECTIONS
{
    .hook : {
        __sym_hook_start = .;
        *(.hook.data)
        __sym_hook_end = .;
    }
}
