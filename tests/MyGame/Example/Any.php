<?php
// automatically generated, do not modify

namespace MyGame\Example;

class Any
{
    const NONE = 0;
    const Monster = 1;
    const TestSimpleTableWithEnum = 2;

    private static $names = array(
        "NONE",
        "Monster",
        "TestSimpleTableWithEnum",
    );

    public static function Name($e)
    {
        if (!isset(self::$names[$e])) {
            throw new \Exception();
        }
        return self::$names[$e];
    }
}
