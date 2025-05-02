<div dir='rtl'>

# حل قضیه EvenDVS برای حالت $\alpha$ های مستقل

مهدی بهرامیان 401171593

## تعریف مسئله

$n$ وظیفه با نام های
$\tau_1 \dots \tau_n$
داریم که هرکدام با نرخ فعالیت
$\alpha_i$
به مدت
$t_i$
طول میکشند و همگی باید تا محدودیت زمانی
$D$
خاتمه یابند.

### خواسته مسئله

ضریب ولتاژ و فرکانس وظایف با نام های
$\rho_i$
را طوری تعیین کنید که در عین حال که محدودیت زمانی $D$ را رعایت میکنیم،
مصرف انرژی به حداقل برسد.

## حل مسئله

$$
E_N = \sum_i \rho_i^2 \alpha_i t_i
\hspace{1cm}
\text{Deadline : }
\sum_i  {t_i \over \rho_i} \leq D
\hspace{0.5cm}
\Rightarrow
\hspace{0.5cm}
\text{Minimize : }
H(\rho_1 \dots \rho_n , \lambda) =
\sum_i \rho_i^2 \alpha_i t_i
+
\lambda( \sum_i  {t_i \over \rho_i} - D )
$$

$$
0 = { \delta H \over \delta \rho_i } =
2 \rho_i \alpha_i t_i +
\lambda { - t_i \over \rho_i^2 }
\Rightarrow
2 \rho_i \alpha_i t_i =
\lambda {  t_i \over \rho_i^2 }
\Rightarrow
2 \rho_i^3 \alpha_i = \lambda
\Rightarrow
\rho_i  = \sqrt[3]{ \lambda \over 2 \alpha_i }
\equiv
{ 1 \over \Lambda } \sqrt[3]{ 1 \over 2 \alpha_i }
\Rightarrow
\Lambda \sum_i t_i \sqrt[3]{ 2 \alpha_i }  = D
$$

$$
\Rightarrow
\Lambda   = { D \over \sum_i t_i \sqrt[3]{ 2 \alpha_i } }
\Rightarrow
\rho_i  =
{  \sum_i t_i \sqrt[3]{ \alpha_i } \over D }
\sqrt[3]{1 \over 2 \alpha_i }
$$

بنابرین پاسخ کلی این مسئله
$
\rho_i  =
{ \sum_i t_i \sqrt[3]{ \alpha_i } \over D }
\sqrt[3]{1 \over 2 \alpha_i }
$

و در نتیجه slack-time 
    هرکدام از وظایف برابر با
    $
    t_i ({1 \over \rho_i} - 1) = t_i (
{ D \over \sum_i t_i \sqrt[3]{ \alpha_i } }
\sqrt[3]{2 \alpha_i } - 1 )
$

</div>
