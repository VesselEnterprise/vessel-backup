<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use Spatie\BinaryUuid\HasBinaryUuid;

class AppClient extends Model
{

  use HasBinaryUuid;

  public function getKeyName()
  {
      return 'id';
  }

  /**
   * The attributes that are mass assignable.
   *
   * @var array
   */
  protected $fillable = [
      'uuid', 'name', 'os', 'dns_name', 'ip_address', 'domain', 'client_version', 'last_check_in'
  ];

  protected $table = 'app_client';

  protected $dates = ['created_at', 'updated_at', 'last_check_in'];

}
