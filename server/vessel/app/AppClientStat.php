<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use Spatie\BinaryUuid\HasBinaryUuid;

class AppClientStat extends Model
{
		use HasBinaryUuid;

		public function client()
		{
				return $this->belongsTo('App\AppClient', 'client_id', 'client_id');
		}

		public function user()
		{
				return $this->belongsTo('App\User', 'user_id', 'user_id');
		}

		protected $table = 'app_client_stat';
		protected $fillable = ['user_id', 'client_id'];
    protected $uuids = ['client_id'];
		protected $dates = ['created_at', 'updated_at'];
}
