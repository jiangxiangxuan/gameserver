<?php
namespace app\api\model;

use think\Db;
use think\Model;

class GameUser extends Model
{
	protected $table = 'gameuser';
	
	public function getUserInfoByUserName($username)
	{
		return Db::query('select * from gameuser where username="'.$username.'" limit 1;');
	}
}
