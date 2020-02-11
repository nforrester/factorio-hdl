(load "circuits/util.scm")

(define sym-for-idx
  (lambda (base-symbol index)
    (strings->symbol (symbol->string base-symbol) (number->string index))))

(define defpart-dual-demux-all-signals-n-circuits
  (lambda (num-circuits)
    `(defpart (strings->symbol "dual-demux-all-signals-" (number->string num-circuits) "-circuits")
       ((in yellow address)
        (in yellow input)
        ,@(for (range num-circuits)
            (lambda (index)
              `(out yellow ,(sym-for-idx output index))))
        (signal address-signal)
        (signal input-signal))

       ; delay 1
       (green which-signal)
       (arithmetic address which-signal address-signal % ,(len all-signals) address-signal)

       ; delay 1
       (green which-circuit)
       (arithmetic address which-circuit address-signal / ,(len all-signals) address-signal)

       ; delay 2
       (green which-circuit-delayed)
       (buffer which-circuit which-circuit-delayed)

       ; delay 1
       (green input-delayed)
       (buffer input input-delayed)

       ; delay 2
       (green input-on-sig)
       (demux-all-signals input-delayed input-on-sig which-signal address-signal input-signal)

       ; delay 3
       ((strings->symbol "demux-" (number->string num-circuits) "-circuits")
        input-on-sig
        ,@(for (range num-circuits)
            (lambda (index)
              (sym-for-idx output index)))
        which-circuit-delayed
        address-signal))))

(define defpart-dual-mux-all-signals-n-circuits
  (lambda (num-circuits)
    `(defpart (strings->symbol "dual-mux-all-signals-" (number->string num-circuits) "-circuits")
       ((in yellow address)
        ,@(for (range num-circuits)
            (lambda (index)
              `(out yellow ,(sym-for-idx input index))))
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
       (green which-signal-delayed)
       (buffer which-signal which-signal-delayed)

       ; delay 1
       ,@(apply append
          (for (range num-circuits)
            (lambda (index)
              `((green ,(sym-for-idx input-delayed index))
                (buffer ,(sym-for-idx input index) ,(sym-for-idx input-delayed index))))))

       ; delay 2
       (green muxed-circuit)
       ((strings->symbol "mux-" (number->string num-circuits) "-circuits")
        ,@(for (range num-circuits)
            (lambda (index)
              (sym-for-idx input index)))
        muxed-circuit
        which-circuit
        address-signal)

       ; delay 3
       (mux-all-signals muxed-circuit output which-signal-delayed address-signal output-signal))))

(define defpart-memory-n
  (lambda (num-cells)
    `(defpart (strings->symbol "memory-" (number->string num-cells))
       ((in yellow address)
        (in yellow write)
        (in yellow data-in)
        (out yellow data-out)
        (signal address-signal)
        (signal write-signal)
        (signal data-in-signal)
        (signal data-out-signal))

       ,@(for (range num-cells)
           (lambda (index)
             `(green ,(sym-for-idx data-in-cell index))))
       ((strings->symbol "dual-demux-all-signals-" (number->string num-cells) "-circuits")
        address
        data-in
        ,@(for (range num-cells)
            (lambda (index)
              (sym-for-idx data-in-cell index)))
        address-signal
        data-in-signal)

       ,@(for (range num-cells)
           (lambda (index)
             `(green ,(sym-for-idx write-cell index))))
       ((strings->symbol "dual-demux-all-signals-" (number->string num-cells) "-circuits")
        address
        write
        ,@(for (range num-cells)
            (lambda (index)
              (sym-for-idx write-cell index)))
        address-signal
        write-signal)

       (red all-ones)
       (constant-all-ones all-ones)

       ,@(apply append
           (for (range num-cells)
             (lambda (index)
               `((green ,(sym-for-idx data-out-cell index))
                 (latch*
                  ,(sym-for-idx data-in-cell index)
                  ,(sym-for-idx data-out-cell index)
                  ,(sym-for-idx write-cell index)
                  all-ones)))))

       ((strings->symbol "dual-mux-all-signals-" (number->string num-cells) "-circuits")
        address
        ,@(for (range num-cells)
            (lambda (index)
              (sym-for-idx data-out-cell index)))
        data-out
        address-signal
        write-signal))))
