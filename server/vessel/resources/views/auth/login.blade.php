@extends('layouts.app')

@section('content')

<div class="ui very padded container">

	<h2 class="ui header">
		Login
		<div class="sub header">Login to your account to access the management portal</div>
	</h2>

	<form method="POST" action="{{ route('login') }}" class="ui form segment">
	  @csrf

			<div class="ui stackable centered grid">
			  <div class="row">
					<div class="two wide column">
				    <label>{{ __('E-Mail Address') }}</label>
					</div>
					<div class="four wide column field">
						<input type="email" id="email" name="email" placeholder="john.doe@email.com" value="{{ old('email') }}" required autofocus>
						@if ($errors->has('email'))
								<span class="invalid-feedback" role="alert">
										<strong>{{ $errors->first('email') }}</strong>
								</span>
						@endif
					</div>
				</div>
				<div class="row">
					<div class="two wide column">
						<label>{{ __('Password') }}</label>
					</div>
					<div class="four wide column field">
						<input type="password" id="password" name="password" autocomplete="off" style="background-image: url(&quot;data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR4nGP6zwAAAgcBApocMXEAAAAASUVORK5CYII=&quot;); cursor: auto;" required>
						@if ($errors->has('password'))
								<span class="invalid-feedback" role="alert">
										<strong>{{ $errors->first('password') }}</strong>
								</span>
						@endif
					</div>
				</div>
				<div class="sixteen wide row">
					<div class="three wide right aligned column field">
						<div class="ui toggle checkbox">
				      <input type="checkbox" name="remember" {{ old('remember') ? 'checked' : '' }} class="hidden">
				      <label>{{ __('Remember Me') }}</label>
			    	</div>
					</div>
				</div>
				<div class="sixteen wide row">
					<div class="one wide centered column">
						<button type="submit" class="ui primary button">
								{{ __('Login') }}
						</button>
					</div>
				</div>
				<div class="sixteen wide row">
					<div class="three wide right aligned column">
						<a class="" href="{{ route('password.request') }}">
								{{ __('Forgot Your Password?') }}
						</a>
					</div>
				</div>
			</div>
	</form>

</div>
@endsection
