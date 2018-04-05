<?php
namespace app\api\controller;

use app\api\model;

class Index extends Base
{
    public function index()
    {
        return '';
    }
	
	// 登录
    public function login($username,$userpasswd)
    {
		$user = new GameUser();
		$userinfo = $user->getUserInfoByUserName($username);
        return '{"state":0,"token":""}';
    }
	
	// 验证token
	public function verifytoken($token)
	{
        return '{"state":0,"data":""}';
	}
}
