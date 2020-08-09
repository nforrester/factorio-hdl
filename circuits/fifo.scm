(load "circuits/util.scm")

; TODO document
; TODO add empty detection, with default value
(define defpart-fifo-n
  (lambda (size)
    `(defpart ,(strings->symbol "fifo-" (number->string size))
       ((in red data-in)
        (out red data-out)
        (in red push)
        (in red pop)
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
       (decider (reset write-ptr) write-ptr reset-signal == 1 address-signal one)

       ; Detect FIFO overrun
       (red read-ptr-check)
       (green write-ptr-check)
       (arithmetic read-ptr read-ptr-check address-signal * 1 sig:signal-a)
       (arithmetic write-ptr write-ptr-check address-signal * 1 sig:signal-b)
       (decider (read-ptr-check write-ptr-check) errors sig:signal-a == sig:signal-b error-signal one)

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
       (green read-ptr-mod)
       (green write-ptr-mod)
       (arithmetic read-ptr  read-ptr-mod  address-signal % ,size address-signal)
       (arithmetic write-ptr write-ptr-mod address-signal % ,size address-signal)
       (decider (push minus-size-green) write-ptr-mod control-signal == 0 address-signal input-count)

       ; delay 1
       (red data-in-delayed1)
       (buffer data-in data-in-delayed1)

       ; The buffer
       (,(strings->symbol "fast-memory-start-0-width-" (number->string size))
         data-in-delayed1 data-out
         write-ptr-mod read-ptr-mod
         data-signal address-signal address-signal))))
