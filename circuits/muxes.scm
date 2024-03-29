(load "circuits/util.scm")

(define defpart-mux-n-circuits-offset-by-m
  (lambda (n offset)
    (let ((in-wires (list-of-n-symbols 'in n)))
      `(defpart ,(strings->symbol "mux-" (number->string n)
                                  "-circuits-offset-by-" (number->string offset))
         (,@(for in-wires (lambda (wire) `(in yellow ,wire)))
          (out yellow o)
          (in yellow control)
          (signal control-sig))

         ,@(for (zip in-wires (range n))
             (lambda+ (in-wire index)
               `(relay-equal ,in-wire o control control-sig ,(+ index offset))))))))

(define defpart-demux-n-circuits-offset-by-m
  (lambda (n offset)
    (let ((out-wires (list-of-n-symbols 'out n)))
      `(defpart ,(strings->symbol "demux-" (number->string n)
                                  "-circuits-offset-by-" (number->string offset))
         ((in yellow i)
          ,@(for out-wires (lambda (wire) `(out yellow ,wire)))
          (in yellow control)
          (signal control-sig))

         ,@(for (zip out-wires (range n))
             (lambda+ (out-wire index)
               `(relay-equal i ,out-wire control control-sig ,(+ index offset))))))))

(define defpart-mux-n-signals
  (lambda (n)
    (let ((signals (list-of-n-symbols 'sig n)))
      `(defpart ,(strings->symbol "mux-" (number->string n) "-signals")
         ((in yellow i)
          (out yellow o)
          (in yellow control)
          (signal control-sig)
          (signal output-sig)
          ,@(for signals (lambda (sig) `(signal ,sig))))

         ; i-most is like i but with sig0 replaced with the value of control-sig from control.
         ; delay 1
         (red i-most)
         (buffer-without-signal i i-most sig0)
         (arithmetic control i-most control-sig * 1 sig0)

         ; i-one has sig0 from i and the value of control-sig
         ; from control moved to a signal other than sig0.
         ; delay 1
         (red i-one)
         (signal not-sig0 (notsigs sig0))
         (buffer-only-signal i i-one sig0)
         (arithmetic control i-one control-sig * 1 not-sig0)

         ; o-wrong-sig has only one active signal, and it has the value we want,
         ; but it's probably not the output signal.
         ; delay 2
         (red o-wrong-sig)
         (decider i-one o-wrong-sig not-sig0 == 0 sig0 input-count)
         ,@(for (cdr (zip signals (range n)))
             (lambda+ (sig index)
               `(decider i-most o-wrong-sig sig0 == ,index ,sig input-count)))

         ; delay 3
         (arithmetic o-wrong-sig o each * 1 output-sig)))))

(define defpart-demux-n-signals
  (lambda (n)
    (let ((signals (list-of-n-symbols 'sig n)))
      `(defpart ,(strings->symbol "demux-" (number->string n) "-signals")
         ((in yellow i)
          (out yellow o)
          (in yellow control)
          (signal control-sig)
          (signal input-sig)
          ,@(for signals (lambda (sig) `(signal ,sig))))

         ; duplicated-input-most is like i but with sig0 replaced with
         ; the value of control-sig from control
         (red duplicated-input-most)
         (arithmetic control duplicated-input-most control-sig * 1 sig0)
         ,@(for (cdr signals) (lambda (sig)
             `(arithmetic i duplicated-input-most input-sig * 1 ,sig)))

         ; duplicated-input-one has sig0 with the value of input-sig from i and the
         ; value of control-sig from control moved to a signal other than sig0.
         (red duplicated-input-one)
         (signal not-sig0 (notsigs sig0))
         (arithmetic i duplicated-input-one input-sig * 1 sig0)
         (arithmetic control duplicated-input-one control-sig * 1 not-sig0)

         (decider duplicated-input-one o not-sig0 == 0 sig0 input-count)

         ,@(for (cdr (zip signals (range n)))
             (lambda+ (sig index)
               `(decider duplicated-input-most o sig0 == ,index ,sig input-count)))))))

(define defpart-mux-all-signals
  (lambda ()
    `(begin
       (defpart-mux-n-signals ,(len all-signals))
       (defpart mux-all-signals
         ((in yellow i)
          (out yellow o)
          (in yellow control)
          (signal control-sig)
          (signal output-sig))

         (,(strings->symbol "mux-" (number->string (len all-signals)) "-signals")
          i o control control-sig output-sig ,@all-signals)))))

(define defpart-demux-all-signals
  (lambda ()
    `(begin
       (defpart-demux-n-signals ,(len all-signals))
       (defpart demux-all-signals
         ((in yellow i)
          (out yellow o)
          (in yellow control)
          (signal control-sig)
          (signal input-sig))

         (,(strings->symbol "demux-" (number->string (len all-signals)) "-signals")
          i o control control-sig input-sig ,@all-signals)))))
