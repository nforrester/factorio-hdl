(load "circuits/util.scm")

(define sym-for-idx
  (lambda (base-symbol index)
    (strings->symbol (symbol->string base-symbol) (number->string index))))

(define defpart-dual-mux-all-signals-n-circuits
  (lambda (num-circuits)
    `(defpart ,(strings->symbol "dual-mux-all-signals-" (number->string num-circuits) "-circuits")
       ((in yellow address)
        ,@(for (range num-circuits)
            (lambda (index)
              `(in yellow ,(sym-for-idx 'input index))))
        (out yellow output)
        (signal address-signal)
        (signal output-signal))

       ; delay 1
       (green which-circuit)
       (arithmetic address which-circuit address-signal / ,(len all-signals) address-signal)

       ; delay 1
       (green which-signal)
       (arithmetic address which-signal address-signal % ,(len all-signals) address-signal)

       ; delay 2
       (green which-signal-delayed1)
       (buffer which-signal which-signal-delayed1)

       ; delay 3
       (green which-signal-delayed2)
       (buffer which-signal-delayed1 which-signal-delayed2)

       ; delay 1
       ,@(apply append
          (for (range num-circuits)
            (lambda (index)
              `((green ,(sym-for-idx 'input-delayed index))
                (buffer ,(sym-for-idx 'input index) ,(sym-for-idx 'input-delayed index))))))

       ; delay 3
       (green muxed-circuit)
       (,(strings->symbol "mux-" (number->string num-circuits) "-circuits")
          ,@(for (range num-circuits)
              (lambda (index)
                (sym-for-idx 'input-delayed index)))
          muxed-circuit
          which-circuit
          address-signal)

       ; delay 6
       (mux-all-signals muxed-circuit output which-signal-delayed2 address-signal output-signal))))

(define defpart-dual-demux-all-signals-n-circuits
  (lambda (num-circuits)
    `(defpart ,(strings->symbol "dual-demux-all-signals-" (number->string num-circuits) "-circuits")
       ((in yellow address)
        (in yellow input)
        ,@(for (range num-circuits)
            (lambda (index)
              `(out yellow ,(sym-for-idx 'output index))))
        (signal address-signal)
        (signal input-signal))

       ; delay 1
       (green which-signal)
       (arithmetic address which-signal address-signal % ,(len all-signals) address-signal)

       ; delay 1
       (green which-circuit)
       (arithmetic address which-circuit address-signal / ,(len all-signals) address-signal)

       ; delay 2
       (green which-circuit-delayed1)
       (buffer which-circuit which-circuit-delayed1)

       ; delay 3
       (green which-circuit-delayed2)
       (buffer which-circuit-delayed1 which-circuit-delayed2)

       ; delay 1
       (green input-delayed)
       (buffer input input-delayed)

       ; delay 3
       (green input-on-sig)
       (demux-all-signals input-delayed input-on-sig which-signal address-signal input-signal)

       ; delay 5
       (,(strings->symbol "demux-" (number->string num-circuits) "-circuits")
          input-on-sig
          ,@(for (range num-circuits)
              (lambda (index)
                (sym-for-idx 'output index)))
          which-circuit-delayed2
          address-signal))))

; Define a memory with (* num-cells (len all-signals)) signed 32 bit int registers.
; To write the value of a register:
;   - Tick 0
;     - Set address to the address of the register you want to set (0 indexed).
;     - Set data-in to the value you wish to set.
;     - Set write to 1.
;     - Hold inputs stable for 5 ticks.
;   - Tick 5
;     - Values may now be changed.
;     - You may not request to read the register until 6 ticks after this point.
;   - Tick 11
;     - You may now submit a read request for this register.
; To read the value of a register:
;   - Tick 0
;     - Set address to the address of the register you want to read (0 indexed).
;     - Set write to 0.
;     - There is no special stability time requirement (1 tick required stability time).
;     - The desired value will appear on data-out 6 ticks later.
;   - Tick 6
;     - The value of the register appears on data-out.
; Read and write transactions may be submitted with no intervening delay
; (their processing is pipelined).
; You may read a register and then immediately write it.
; If you write a register you must wait 5 ticks before trying to read it again.
(define defpart-memory-n
  (lambda (num-cells)
    (let ((dual-mux-all-signals-n-circuits
            (strings->symbol "dual-mux-all-signals-" (number->string num-cells) "-circuits"))
          (dual-demux-all-signals-n-circuits
            (strings->symbol "dual-demux-all-signals-" (number->string num-cells) "-circuits")))
      `(defpart ,(strings->symbol "memory-" (number->string num-cells))
         ((in yellow address)
          (in yellow write)
          (in yellow data-in)
          (out yellow data-out)
          (signal address-signal)
          (signal write-signal)
          (signal data-in-signal)
          (signal data-out-signal))

         ; Demux data-in to the registers.
         ,@(for (range num-cells)
             (lambda (index)
               `(green ,(sym-for-idx 'data-in-cell index))))
         (,dual-demux-all-signals-n-circuits
            address
            data-in
            ,@(for (range num-cells)
                (lambda (index)
                  (sym-for-idx 'data-in-cell index)))
            address-signal
            data-in-signal)

         ; Demux write to the registers.
         ,@(for (range num-cells)
             (lambda (index)
               `(green ,(sym-for-idx 'write-cell index))))
         (,dual-demux-all-signals-n-circuits
            address
            write
            ,@(for (range num-cells)
                (lambda (index)
                  (sym-for-idx 'write-cell index)))
            address-signal
            write-signal)

         ; Helper for latch*
         (red all-ones)
         (constant-all-ones all-ones)

         ; The registers.
         ,@(apply append
             (for (range num-cells)
               (lambda (index)
                 `((green ,(sym-for-idx 'data-out-cell index))
                   (latch*
                    ,(sym-for-idx 'data-in-cell index)
                    ,(sym-for-idx 'data-out-cell index)
                    ,(sym-for-idx 'write-cell index)
                    all-ones)))))

         ; Mux data-out from the registers.
         (,dual-mux-all-signals-n-circuits
            address
            ,@(for (range num-cells)
                (lambda (index)
                  (sym-for-idx 'data-out-cell index)))
            data-out
            address-signal
            data-out-signal)))))