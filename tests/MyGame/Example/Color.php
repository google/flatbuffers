<?php
// automatically generated, do not modify

namespace MyGame\Example;

class Color
{
    const Red = 1;
    const Green = 2;
    const Blue = 8;

    private static $names = array(
        "Red",
        "Green",
        "Blue",
    );

    public static function Name($e)
    {
        if (!isset(self::$names[$e])) {
            throw new \Exception();
        }
        return self::$names[$e];
    }
}
