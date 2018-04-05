<?php
namespace app\api\controller;

class Index extends Base
{
    public function index()
    {
        return '';
    }
	
	// 登录
    public function login($username,$userpasswd)
    {
        return '{"state":0,"token":""}';
    }
	
	// 验证token
	public function verifytoken($token)
	{
        return '{"state":0,"data":""}';
	}
}
