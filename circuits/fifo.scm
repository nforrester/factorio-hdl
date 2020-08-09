(load "circuits/util.scm")

; TODO document
; TODO instead of popping when empty being an error, just make it a nop and keep returning the default value.
(define defpart-fifo-n
  (lambda (size)
    `(defpart ,(strings->symbol "fifo-" (number->string size))
       ((in red data-in)
        (out red data-out)
        (in red push)
        (in red pop)
        (in red default-value)
        (in red reset)
        (out yellow errors)
        (signal data-signal)
        (signal control-signal)
        (signal reset-signal)
        (signal error-signal))

       ; Write and read pointer addresses are modular.
       ; 0 is the same as size, 1 is the same as size + 1, etc.
       ; This relaxes the requirement to wrap them around at the exactly correct moment,
       ; or exactly once per revolution through the buffer.
       ; It is still important to wrap them because otherwise they'd grow without bound
       ; and eventually overflow. Yes, that would require more than two years of continuous
       ; operation, but it's the principle of the thing.
       (signal address-signal (notsigs data-signal control-signal reset-signal))
       (green write-ptr)
       (green read-ptr)

       (red minus-size-red)
       (green minus-size-green)
       (constant (minus-size-red minus-size-green) ((address-signal ,(* -1 size))))

       ; Preserve pointer values, but allow reset
       (decider (reset read-ptr) read-ptr reset-signal == 0 address-signal input-count)
       (decider (reset write-ptr) write-ptr reset-signal == 0 address-signal input-count)

       ; Increment pointers on push and pop
       (decider push write-ptr control-signal == 1 address-signal one)
       (decider pop read-ptr control-signal == 1 address-signal one)

       ; Wrap around to prevent overflow. This doesn't guarantee anything about the
       ; timing of the wrapping, only that it will happen well before integer
       ; overflow and that at any given moment the pointer values will be correct
       ; modulo the size of the fifo.
       (signal need-to-wrap-sig sig:signal-w)
       (green need-to-wrap-write)
       (green need-to-wrap-read)
       (decider write-ptr need-to-wrap-write address-signal > ,(* 10 size) need-to-wrap-sig one)
       (decider read-ptr  need-to-wrap-read  address-signal > ,(* 10 size) need-to-wrap-sig one)
       (decider (need-to-wrap-write minus-size-red) write-ptr need-to-wrap-sig == 1 address-signal input-count)
       (decider (need-to-wrap-read  minus-size-red) read-ptr  need-to-wrap-sig == 1 address-signal input-count)

       ; delay 1
       ; 0 <= write-ptr-mod < size, unless push == 0
       (signal address-check-signal (notsigs address-signal))
       (green read-ptr-mod)
       (red write-ptr-mod-check)
       (green write-ptr-mod-maybe)
       (arithmetic read-ptr  read-ptr-mod  address-signal % ,size address-signal)
       (arithmetic write-ptr write-ptr-mod-check address-signal % ,size address-check-signal)
       (arithmetic write-ptr write-ptr-mod-maybe address-signal % ,size address-signal)
       (decider (push minus-size-green) write-ptr-mod-maybe control-signal == 0 address-signal input-count)

       ; delay 1
       (red data-in-delayed1)
       (buffer data-in data-in-delayed1)

       ; delay 2
       ; Detect empty
       (green empty)
       (decider (read-ptr-mod write-ptr-mod-check) empty address-signal == address-check-signal control-signal one)

       ; delay 2
       ; The buffer
       (red data-out-raw)
       (,(strings->symbol "fast-memory-start-0-width-" (number->string size))
         data-in-delayed1 data-out-raw
         write-ptr-mod-maybe read-ptr-mod
         data-signal address-signal address-signal)

       ; delay 3
       (decider (data-out-raw empty) data-out control-signal == 0 data-signal input-count)
       (decider (default-value empty) data-out control-signal == 1 data-signal input-count)

       ; Detect FIFO overrun
       ; empty is delayed 2
       ; pop-plus-not-push is delayed 1
       ; We are checking whether we are popping and not pushing when previously empty,
       ; meaning that we're causing a FIFO overrun.
       (red pop-plus-not-push)
       (decider pop  pop-plus-not-push control-signal == 1 control-signal one)
       (decider push pop-plus-not-push control-signal == 0 control-signal one)
       (decider (pop-plus-not-push empty) errors control-signal == 3 error-signal one)

       )))
