(defrule rul-001-infraredalarm
    ?sct <- (object (is-a SceneContext)
         (where room0) (target infraredalarm) (action ?action)
         (services $?svrs)
    )
    ?light <- (object (is-a oic.d.light)
        (ID ?did &:(eq ?did ins-00a17a88-a01a-02a1-4af9)))
  =>
    (bind ?c (create-rule-context rul-001-infraredalarm))
    (logi "scene smokealarm, action: " ?action)
    (if (eq ?action exit)
     then
        (foreach ?sname (create$ $?svrs)
            (logd "stop service: " ?sname)
            (assert (stop-service ?sname))
        )
        (return)
    )
    (if (eq ?action enter)
     then
        (bind ?etime (+ 30000 (nth$ 1 (now))))
        (bind ?hue 260)
        (bind ?sat 350)
        (assert (start-service ?sct alarmlight:1 ?etime ?did ?hue ?sat))
    )
)
