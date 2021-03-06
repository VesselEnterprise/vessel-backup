@extends('layouts.app')

@section('scripts')

<script type="text/javascript">

	$(document).ready(function() {

		$('.message .close')
			.on('click', function() {
				$(this)
					.closest('.message')
					.transition('fade');
			});

	});

</script>

@endsection

@section('content')

<div class="ui very padded container">

	<h2 class="ui header">
		Login
		<div class="sub header">Login to your account to access the management portal</div>
	</h2>

	<form method="POST" action="{{ route('login') }}" class="ui form segment">
	  @csrf

			<div class="ui padded stackable centered grid">
				@if(isset($error))
					<div class="ui small negative message">
						<i class="close icon"></i>
						<div class="header">
							{{ $error }}
						</div>
					</div>
				@endif
			  <div class="row">
					<div class="two wide column">
				    <label>{{ __('E-Mail or Username') }}</label>
					</div>
					<div class="four wide column field">
						<input type="text" id="username" name="username" placeholder="john.doe@email.com" value="{{ old('username') }}" required autofocus>
						@if ($errors->has('username'))
								<span class="invalid-feedback" role="alert">
										<strong>{{ $errors->first('username') }}</strong>
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
