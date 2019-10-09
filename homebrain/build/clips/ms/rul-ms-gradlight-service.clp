;=================================================================
; date: 2018-07-15 21:00:00
; title: gradlight micro service
; author: QRS
;=================================================================

(defrule rul-ms-gradlight-service
    (datetime ?clock $?ox)
    ?svr <- (service
        (name ?name) (skey ?key &:(eq ?key ?name))
        (state ?st) (saves $?saves)
        (args ?ud ?light ?cnt ?timelen $?oy)
        (ntime ?tm &:(or (> ?clock ?tm) (neq ?st running)))
    )
    ?ins <- (object (is-a oic.d.light)
        (ID ?did &:(eq ?did ?light))
        (brightness ?brg ?brg-old) (value ?val ?val-old)
    )
  =>
    (logi "service: " ?name ", skey: " ?key ", state: " ?st)
    (if (eq ?st stop)
     then
        (logi "service: " ?name " stop")
        (return)
    )
    (if (eq ?ud up)
     then
        (if (eq ?st start)
         then
            (if (< ?timelen ?cnt)
             then
                (act-control ?did brightness 100)
                (return)
            )
            (bind ?deltatime (integer (/ ?timelen ?cnt)))
            (if (= ?val 0)
             then
                (act-control ?did value 1)
                (act-control ?did brightness 0)
                (bind ?deltabrg (integer (/ 100 ?cnt)))
             else
                (bind ?deltabrg (integer (/ (- 100 ?brg) ?cnt)))
            )
            (modify ?svr (state running) (ntime (+ ?clock ?deltatime)) (saves ?clock ?brg ?deltatime ?deltabrg))
            (return)
        )
        (if (>= ?brg 100)
         then
            (assert (stop-service ?name))
            (return)
        )
        (bind ?starttime (nth$ 1 ?saves))
        (bind ?startbrg (nth$ 2 ?saves))
        (bind ?deltatime (nth$ 3 ?saves))
        (bind ?co (integer (/ (- ?clock ?starttime) ?deltatime)))
        (if (> ?co 0)
         then
            (bind ?deltabrg (nth$ 4 ?saves))
            (bind ?target (+ ?startbrg (* ?deltabrg ?co)))
            (if (< ?target 100)
             then
                (act-control ?did brightness ?target)
             else
                (act-control ?did brightness 100)
            )
            (modify ?svr (ntime (+ ?clock ?deltatime)))
        )
     else
        (if (eq ?st start)
         then
            (if (< ?timelen ?cnt)
             then
                (act-control ?did brightness 0)
                (return)
            )
            (bind ?deltatime (integer (/ ?timelen ?cnt)))
            (bind ?deltabrg (integer (/ ?brg ?cnt)))
            (modify ?svr (state running) (ntime (+ ?clock ?deltatime)) (saves ?clock ?brg ?deltatime ?deltabrg))
            (return)
        )
        (if (<= ?brg 0)
         then
            (act-control ?did value 0)
            (assert (stop-service ?name))
            (return)
        )
        (bind ?starttime (nth$ 1 ?saves))
        (bind ?startbrg (nth$ 2 ?saves))
        (bind ?deltatime (nth$ 3 ?saves))
        (bind ?co (integer (/ (- ?clock ?starttime) ?deltatime)))
        (if (> ?co 0)
         then
            (bind ?deltabrg (nth$ 4 ?saves))
            (bind ?target (- ?startbrg (* ?deltabrg ?co)))
            (if (> ?target 0)
             then
                (act-control ?did brightness ?target)
             else
                (act-control ?did brightness 0)
            )
            (modify ?svr (ntime (+ ?clock ?deltatime)))
        )
    )
)
