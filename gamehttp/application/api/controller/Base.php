<?php
namespace app\api\controller;

use think\Controller;
use think\facade\Request;
use think\facade\Config;

class Base extends Controller
{
    public function _initialize()
    {
        parent::_initialize();
        
        // 验证签名
        if(!$this->CheckSign())
        {
            $baseUrl = url('Index/index');
            $this->redirect($baseUrl);
        }
    }

	// 生成token
	public function token()
	{
		$str = md5(uniqid(md5(microtime(true)),true));
        $str = sha1($str);
        return $str;
	}

    // 验证签名
    public function CheckSign() 
    {
        $requests=Request::instance()->param();

        $csign="";
        $argArr = array();
        foreach($requests as $key=>$value)  
        {
            if($key=="sign")
            {
                $csign = $value;
                continue;
            }
            $argArr[$key] = $value;
        }

        ksort($argArr);

        $requestString = '';
        foreach($argArr as $k => $v) 
        {
            $requestString .= $k . '=' . urlencode($v);//."&";
        }

        $requestString .= "key=".Config::get("Config.signkey");

        if(md5($requestString) == strtolower($csign))
        {
            return true;
        }
        return true;
    }

}
