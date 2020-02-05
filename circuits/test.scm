(load "circuits/util.scm")

(define gen-demux-signals
  (lambda ()
    (let ((sig0 (car all-signals)))
      `(defpart demux-signals
         ((in yellow i)
          (out yellow o)
          (in yellow control)
          (signal control-sig)
          (signal input-sig))
      
         (red duplicated-input)
         ,@(map (lambda (sig)
                  `(arithmetic i duplicated-input input-sig * 1 ,sig))
                all-signals)
      
         ; duplicated-input-most is like duplicated-input but with sig0 replaced with
         ; the value of control-sig from control
         (red duplicated-input-most)
         (buffer-without-signal duplicated-input duplicated-input-most ,sig0)
         (arithmetic control duplicated-input-most control-sig * 1 ,sig0)
      
         ; duplicated-input-one has sig0 from duplicated-input and the value of control-sig
         ; from control moved to a signal other than sig0.
         (red duplicated-input-one)
         (signal not-sig0 (notsigs ,sig0))
         (buffer-only-signal duplicated-input duplicated-input-one ,sig0)
         (arithmetic control duplicated-input-one control-sig * 1 not-sig0)
      
         (decider duplicated-input-one o not-sig0 == 0 ,sig0 input-count)

         ,@(map (lambda (sig)
                  `(decider duplicated-input-most o ,sig0 == 1 ,sig input-count))
                all-signals)))))
